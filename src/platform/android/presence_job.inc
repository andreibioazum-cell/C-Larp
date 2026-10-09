/* JNI entry for com.cb4.PresenceJobService. It runs on the job's worker thread,
 * which may start in a fresh process where android_main() never ran, so it
 * sets up the VM and the data path itself. Returns net_presence_check() codes. */
JNIEXPORT jint JNICALL Java_com_cb4_PresenceJobService_nativePresence(JNIEnv *env, jclass cls, jstring data_dir) {
    JavaVM *vm = NULL;
    const char *dir;
    char buf[320];
    (void)cls;
    if (!env || !data_dir || (*env)->GetJavaVM(env, &vm) != JNI_OK || !vm)
        return -2;
    net_set_java_vm(vm);
    dir = (*env)->GetStringUTFChars(env, data_dir, NULL);
    if (!dir)
        return -2;
    snprintf(buf, sizeof(buf), "%s", dir);
    (*env)->ReleaseStringUTFChars(env, data_dir, dir);
    return (jint)net_presence_check(buf);
}
