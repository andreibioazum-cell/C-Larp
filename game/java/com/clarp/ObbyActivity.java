/*
 * The package is com.clarp, in AndroidManifest.xml and in the JNI names of
 * runtime.c (Java_com_clarp_ObbyActivity_*). All three must agree: if one of them
 * differs, System.loadLibrary still works, but the native editor methods
 * (nativeReplaceText, nativeSubmitText, nativeKeyboardHidden) are not found,
 * nativeReady stays false, and the keyboard opens while nothing reaches the input
 * field.
 *
 * The source path is kept independent from the declared package; javac places
 * the class in classes/com/clarp/ObbyActivity.class.
 */
package com.clarp;

import android.app.NativeActivity;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextWatcher;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.FrameLayout;

/**
 * NativeActivity with a real, focusable Android text editor used only as the
 * input connection for in-game fields. The game continues to draw the field
 * itself; this 1-pixel editor makes every soft IME deliver commitText events.
 *
 * NativeActivity's surface steals view-focus after IME-driven resizes. If the
 * editor loses focus, the keyboard stays on screen but typed characters go
 * nowhere. wantKeyboard stays true until the game hides the IME, and we
 * reclaim focus whenever the native surface takes it away.
 */
public final class ObbyActivity extends NativeActivity {
    /*
     * NativeActivity loads the game .so with dlopen(), which does not register
     * it with the Java runtime: without an explicit System.loadLibrary the
     * first call to any native method below threw UnsatisfiedLinkError and
     * crashed the app the moment the keyboard was opened.
     */
    private static boolean nativeReady;
    static {
        try {
            System.loadLibrary("obby_game");
            nativeReady = true;
        } catch (UnsatisfiedLinkError error) {
            nativeReady = false;
        }
    }

    private EditText nicknameEditor;
    private boolean syncingFromNative;
    private boolean keyboardWasVisible;
    /* Game asked for the IME. Stays true across transient focus losses. */
    private volatile boolean wantKeyboard;
    /* Read from the game thread: while true this editor owns the whole text. */
    private volatile boolean editorActive;
    /* Whether the IME is on screen, judged by the real screen size in onGlobalLayout. */
    private volatile boolean imeLooksVisible;
    private int showAttempts;
    /* A held Backspace gives a run of quick deletions from key repeat, and after
     * several in a row the whole text is cleared at once. */
    private long lastDeleteAt;
    private int deleteStreak;
    private boolean pendingDelete;

    private native void nativeReplaceText(String text);
    private native void nativeSubmitText();
    private native void nativeKeyboardHidden();

    private void replaceTextNative(String text) {
        if (nativeReady) try { nativeReplaceText(text); } catch (UnsatisfiedLinkError ignored) { }
    }
    private void submitTextNative() {
        if (nativeReady) try { nativeSubmitText(); } catch (UnsatisfiedLinkError ignored) { }
    }
    private void keyboardHiddenNative() {
        if (nativeReady) try { nativeKeyboardHidden(); } catch (UnsatisfiedLinkError ignored) { }
    }

    /**
     * Keeps the game surface fullscreen. The legacy flags are supported by the
     * complete Android 10–14 range targeted by this APK; sticky mode lets a
     * swipe reveal system bars only temporarily.
     */
    @SuppressWarnings("deprecation")
    private void enterImmersiveMode() {
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        /* The display refresh rate is deliberately left alone: on TECNO it is not
         * locked to 30 Hz, the game really swings between 30 and 40 fps, and asking
         * for exactly 60 only gets in the way of Android picking a panel mode. */
        enterImmersiveMode();

        nicknameEditor = new EditText(this);
        nicknameEditor.setSingleLine(true);
        // Transparent text with alpha=1: at alpha=0 Gboard and the system IME
        // treat the field as dead and hand over no characters at all.
        nicknameEditor.setTextColor(Color.TRANSPARENT);
        nicknameEditor.setHintTextColor(Color.TRANSPARENT);
        nicknameEditor.setBackgroundColor(Color.TRANSPARENT);
        nicknameEditor.setCursorVisible(false);
        nicknameEditor.setAlpha(1f);
        nicknameEditor.setGravity(Gravity.TOP | Gravity.START);
        nicknameEditor.setFocusable(true);
        nicknameEditor.setFocusableInTouchMode(true);
        nicknameEditor.setClickable(false);
        nicknameEditor.setLongClickable(false);
        /* While the editor is visible it covers the thin strip of the native
         * surface, so a tap there counts as a tap outside the game field and closes
         * the IME instead of returning focus to it. */
        nicknameEditor.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, android.view.MotionEvent event) {
                if (event.getAction() == android.view.MotionEvent.ACTION_DOWN) {
                    hideGameKeyboard();
                }
                return true;
            }
        });
        // VISIBLE_PASSWORD used to sit here: it disables composing in Gboard, so
        // every Latin letter is committed at once, but it also switches on the
        // password layout with a number row the user does not get elsewhere. The
        // text filter gives the same direct committing without composing and keeps
        // the ordinary layout.
        nicknameEditor.setInputType(InputType.TYPE_CLASS_TEXT
                | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
                | InputType.TYPE_TEXT_VARIATION_FILTER);
        nicknameEditor.setImeOptions(EditorInfo.IME_ACTION_DONE
                | EditorInfo.IME_FLAG_NO_EXTRACT_UI
                | EditorInfo.IME_FLAG_NO_FULLSCREEN);
        nicknameEditor.setFilters(new InputFilter[] { new InputFilter.LengthFilter(95) });
        nicknameEditor.setVisibility(View.INVISIBLE);

        /* A full-width strip at the top: a tiny 1x1 field or one in a corner reads
         * as dead and receives nothing. Touches still reach the native InputQueue. */
        float density = getResources().getDisplayMetrics().density;
        int editorH = Math.max(48, (int) (48f * density));
        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT, editorH, Gravity.TOP);
        addContentView(nicknameEditor, params);

        nicknameEditor.addTextChangedListener(new TextWatcher() {
            @Override public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                // A real deletion from the keyboard: the text got shorter rather
                // than replaced wholesale by a sync from the native side.
                pendingDelete = !syncingFromNative && count > 0 && after == 0;
            }
            @Override public void onTextChanged(CharSequence s, int start, int before, int count) { }
            @Override public void afterTextChanged(Editable value) {
                if (syncingFromNative) return;
                replaceTextNative(value.toString());
                if (pendingDelete) {
                    long now = android.os.SystemClock.elapsedRealtime();
                    deleteStreak = (now - lastDeleteAt <= 200) ? deleteStreak + 1 : 1;
                    lastDeleteAt = now;
                    pendingDelete = false;
                    /* A held Backspace: after six deletions in a row, no slower
                     * than 200 ms apart, the rest is cleared at once. Ordinary quick
                     * taps remove one character and never reach the threshold. */
                    if (deleteStreak >= 6 && nicknameEditor.length() > 0) {
                        deleteStreak = 0;
                        nicknameEditor.post(new Runnable() {
                            @Override public void run() {
                                if (nicknameEditor.length() > 0) nicknameEditor.setText("");
                            }
                        });
                    }
                } else {
                    deleteStreak = 0;
                }
            }
        });
        nicknameEditor.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View view, boolean focused) {
                if (focused) {
                    editorActive = nativeReady && wantKeyboard
                            && nicknameEditor.getVisibility() == View.VISIBLE;
                    return;
                }
                /* Native surface often steals focus after adjustResize.
                 * If the game still wants the keyboard, take focus back
                 * instead of marking the editor dead — otherwise the IME
                 * stays up and typed characters never reach the field. */
                if (wantKeyboard && nicknameEditor.getVisibility() == View.VISIBLE) {
                    nicknameEditor.post(new Runnable() {
                        @Override public void run() { claimEditorFocus(); }
                    });
                } else {
                    editorActive = false;
                }
            }
        });
        nicknameEditor.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView view, int actionId, KeyEvent event) {
                boolean enter = actionId == EditorInfo.IME_ACTION_SEND
                        || actionId == EditorInfo.IME_ACTION_DONE
                        || (event != null && event.getKeyCode() == KeyEvent.KEYCODE_ENTER
                            && event.getAction() == KeyEvent.ACTION_DOWN);
                if (enter) {
                    submitTextNative();
                    return true;
                }
                return false;
            }
        });

        // Android does not send a direct callback when the user dismisses an IME
        // with the system Back gesture. Track an actual visible->hidden transition;
        // importantly, do not report "hidden" during the short show request delay.
        // When the keyboard is swiped away the game is told ALWAYS, even if the
        // field still wants input, because otherwise the native visibility flag got
        // stuck on "open": a second tap on the field thought the IME was already up,
        // never reopened it, and input went nowhere, which is what broke the nick
        // field.
        nicknameEditor.getRootView().getViewTreeObserver().addOnGlobalLayoutListener(
                new ViewTreeObserver.OnGlobalLayoutListener() {
                    @Override
                    public void onGlobalLayout() {
                        View root = nicknameEditor.getRootView();
                        Rect visible = new Rect();
                        root.getWindowVisibleDisplayFrame(visible);
                        boolean keyboardVisible = root.getHeight() - visible.bottom
                                > root.getHeight() * 0.15f;
                        imeLooksVisible = keyboardVisible;
                        if (wantKeyboard) claimEditorFocus();
                        if (keyboardVisible) {
                            keyboardWasVisible = true;
                        } else if (keyboardWasVisible) {
                            keyboardWasVisible = false;
                            keyboardHiddenNative();
                        }
                    }
                });
    }

    private void claimEditorFocus() {
        if (nicknameEditor == null || !wantKeyboard) return;
        if (nicknameEditor.getVisibility() != View.VISIBLE) nicknameEditor.setVisibility(View.VISIBLE);
        if (!nicknameEditor.hasFocus()) nicknameEditor.requestFocus();
        editorActive = nativeReady;
    }

    /** Called from native code. It is safe to call from the native game thread. */
    public void showGameKeyboard(final String currentText) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (nicknameEditor == null) return;
                wantKeyboard = true;
                nicknameEditor.setVisibility(View.VISIBLE);
                nicknameEditor.bringToFront();
                replaceEditorText(currentText);
                claimEditorFocus();
                getWindow().setSoftInputMode(
                        WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE
                                | WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
                if (imeLooksVisible) {
                    /* The IME is already on screen: keep the focus, restart nothing. */
                    claimEditorFocus();
                    return;
                }
                showAttempts = 0;
                requestShowWhenReady();
            }
        });
    }

    /* showSoftInput quietly returns false until the editor becomes the target of
     * the IME, since focus and the input connection settle only on the next layout
     * pass after setVisibility(VISIBLE) and requestFocus. The first request is
     * therefore delayed, and while the keyboard has not really appeared, judged by
     * the shrunken screen in onGlobalLayout, the request repeats. */
    private void requestShowWhenReady() {
        if (nicknameEditor == null) return;
        final int attempt = showAttempts++;
        if (attempt >= 10) return;
        nicknameEditor.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (nicknameEditor == null || !wantKeyboard || imeLooksVisible) return;
                claimEditorFocus();
                InputMethodManager input = (InputMethodManager)
                        getSystemService(Context.INPUT_METHOD_SERVICE);
                if (input != null) {
                    if (!input.isActive(nicknameEditor)) nicknameEditor.requestFocus();
                    input.showSoftInput(nicknameEditor, InputMethodManager.SHOW_FORCED);
                }
                requestShowWhenReady();
            }
        }, attempt == 0 ? 60 : 120);
    }

    /** Keeps the hidden editor in sync after the native Send button clears it. */
    public void setGameKeyboardText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                replaceEditorText(text);
                if (wantKeyboard) claimEditorFocus();
            }
        });
    }

    /**
     * Called from the native game thread: true when this editor owns the text,
     * so the native side must not append key events into its own buffer.
     */
    public boolean gameKeyboardActive() {
        return wantKeyboard;
    }

    /** Called from native code when the nickname screen is closed. */
    public void hideGameKeyboard() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (nicknameEditor == null) return;
                wantKeyboard = false;
                InputMethodManager input = (InputMethodManager)
                        getSystemService(Context.INPUT_METHOD_SERVICE);
                if (input != null) {
                    input.hideSoftInputFromWindow(nicknameEditor.getWindowToken(), 0);
                }
                editorActive = false;
                imeLooksVisible = false;
                showAttempts = 10; /* stop the pending show retries */
                getWindow().setSoftInputMode(
                        WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
                nicknameEditor.clearFocus();
                nicknameEditor.setVisibility(View.INVISIBLE);
                /* The text lives in the native buffer, which closing the IME must
                 * not wipe: the game clears it itself after sending or leaving the
                 * screen. */
                keyboardHiddenNative();
            }
        });
    }

    private void replaceEditorText(String text) {
        if (nicknameEditor == null) return;
        String safe = text == null ? "" : text;
        if (safe.contentEquals(nicknameEditor.getText())) return;
        boolean shrinking = safe.length() < nicknameEditor.length();
        syncingFromNative = true;
        nicknameEditor.setText(safe);
        nicknameEditor.setSelection(nicknameEditor.length());
        syncingFromNative = false;
        // restartInput only on a deletion or a clear: otherwise the IME drops the
        // Latin letter just typed and the nick field stays empty.
        if (shrinking) {
            InputMethodManager input = (InputMethodManager)
                    getSystemService(Context.INPUT_METHOD_SERVICE);
            if (input != null) input.restartInput(nicknameEditor);
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        /* Back from the launcher or the recents screen the system bars are shown
         * again; without hiding them the window shrinks and resizes once more
         * right after the return. */
        if (hasFocus) enterImmersiveMode();
        if (hasFocus && wantKeyboard && nicknameEditor != null) {
            claimEditorFocus();
            if (!imeLooksVisible) {
                showAttempts = 0;
                requestShowWhenReady();
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        enterImmersiveMode();
    }

    @Override
    protected void onPause() {
        hideGameKeyboard();
        super.onPause();
    }
}
