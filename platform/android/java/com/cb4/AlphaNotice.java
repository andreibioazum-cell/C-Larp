package com.cb4;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;

public final class AlphaNotice {
    private AlphaNotice() { }

    private static final String TITLE_EN = "Cubic Battle 4 \u2014 alpha";
    private static final String TITLE_RU = "Cubic Battle 4 \u2014 \u0430\u043b\u044c\u0444\u0430";

    private static final String TEXT_EN =
            "This game is an alpha version.\n\n"
            + "Expect bugs, unfinished content, and balance or progress that may "
            + "change between updates. Thanks for playing and helping to make it better!";

    private static final String TEXT_RU =
            "\u0418\u0433\u0440\u0430 \u043d\u0430\u0445\u043e\u0434\u0438\u0442\u0441\u044f \u0432 \u0430\u043b\u044c\u0444\u0430-\u0432\u0435\u0440\u0441\u0438\u0438.\n\n"
            + "\u0417\u0434\u0435\u0441\u044c \u0435\u0441\u0442\u044c \u0431\u0430\u0433\u0438, \u0447\u0430\u0441\u0442\u044c \u043a\u043e\u043d\u0442\u0435\u043d\u0442\u0430 \u0435\u0449\u0451 \u043d\u0435 \u0433\u043e\u0442\u043e\u0432\u0430, \u0430 \u0431\u0430\u043b\u0430\u043d\u0441 "
            + "\u0438 \u043f\u0440\u043e\u0433\u0440\u0435\u0441\u0441 \u043c\u043e\u0433\u0443\u0442 \u043c\u0435\u043d\u044f\u0442\u044c\u0441\u044f \u043c\u0435\u0436\u0434\u0443 \u043e\u0431\u043d\u043e\u0432\u043b\u0435\u043d\u0438\u044f\u043c\u0438. \u0421\u043f\u0430\u0441\u0438\u0431\u043e, "
            + "\u0447\u0442\u043e \u0438\u0433\u0440\u0430\u0435\u0442\u0435 \u0438 \u043f\u043e\u043c\u043e\u0433\u0430\u0435\u0442\u0435 \u0435\u0451 \u0434\u0435\u043b\u0430\u0442\u044c \u043b\u0443\u0447\u0448\u0435!";

    private static final String OK_EN = "OK";
    private static final String OK_RU = "\u041e\u041a";

    public static void show(Activity activity, boolean russian, final Runnable onClosed) {
        if (activity == null || activity.isFinishing()) return;
        new AlertDialog.Builder(activity)
                .setTitle(russian ? TITLE_RU : TITLE_EN)
                .setMessage(russian ? TEXT_RU : TEXT_EN)
                .setCancelable(false)
                .setPositiveButton(russian ? OK_RU : OK_EN, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (onClosed != null) onClosed.run();
                    }
                })
                .show();
    }
}
