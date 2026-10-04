#ifdef __ANDROID__
static int kb_call_int_method(const char *name, int value) {
    int attached = 0, ok = 0;
    JNIEnv *env = kb_get_env(&attached);
    if (!env || !kb_activity || !kb_activity->clazz)
        return 0;
    if ((*env)->PushLocalFrame(env, 4) != 0) {
        kb_release_env(attached);
        return 0;
    }
    jclass cls = (*env)->GetObjectClass(env, kb_activity->clazz);
    jmethodID method = cls ? (*env)->GetMethodID(env, cls, name, "(I)V") : NULL;
    if (method) {
        (*env)->CallVoidMethod(env, kb_activity->clazz, method, (jint)value);
        ok = !(*env)->ExceptionCheck(env);
    }
    if ((*env)->ExceptionCheck(env))
        (*env)->ExceptionClear(env);
    (*env)->PopLocalFrame(env, NULL);
    kb_release_env(attached);
    return ok;
}

void alpha_notice_show(double russian) { (void)kb_call_int_method("showAlphaNotice", russian != 0 ? 1 : 0); }
#else
void alpha_notice_show(double russian) { (void)russian; }
#endif
