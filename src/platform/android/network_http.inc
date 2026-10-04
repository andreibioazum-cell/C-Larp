void net_set_java_vm(JavaVM *vm) {
    net.vm = vm;
}
static int http_ex(const char *method, const char *url, const char *body, char *out, size_t cap, const char *header,
                   const char *value, char *etag, size_t etag_cap) {
    JNIEnv *env = NULL;
    jobject conn = NULL, stream = NULL, urlobj = NULL;
    jclass urlc, connc, streamc;
    jbyteArray buf;
    jstring ju, jm;
    int code = 0, attached = 0, ok = 0;
    size_t total = 0;
    if (out && cap)
        out[0] = '\0';
    if (etag && etag_cap)
        etag[0] = '\0';
    if (!net.vm)
        return 0;
    if ((*net.vm)->GetEnv(net.vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        if ((*net.vm)->AttachCurrentThread(net.vm, &env, NULL) != JNI_OK)
            return 0;
        attached = 1;
    }
#define JNI_CHECK()                                                                                                    \
    do {                                                                                                               \
        if ((*env)->ExceptionCheck(env))                                                                               \
            goto done;                                                                                                 \
    } while (0)
    if ((*env)->PushLocalFrame(env, 32) != 0)
        goto done;
    urlc = (*env)->FindClass(env, "java/net/URL");
    JNI_CHECK();
    connc = (*env)->FindClass(env, "java/net/HttpURLConnection");
    JNI_CHECK();
    ju = (*env)->NewStringUTF(env, url);
    JNI_CHECK();
    urlobj = (*env)->NewObject(env, urlc, (*env)->GetMethodID(env, urlc, "<init>", "(Ljava/lang/String;)V"), ju);
    JNI_CHECK();
    conn = (*env)->CallObjectMethod(env, urlobj,
                                    (*env)->GetMethodID(env, urlc, "openConnection", "()Ljava/net/URLConnection;"));
    JNI_CHECK();
    {

        const char *send_method = method;
        const char *override = NULL;
        if (!strcmp(method, "PATCH")) {
            send_method = "POST";
            override = "PATCH";
        }
        jm = (*env)->NewStringUTF(env, send_method);
        JNI_CHECK();
        (*env)->CallVoidMethod(env, conn, (*env)->GetMethodID(env, connc, "setRequestMethod", "(Ljava/lang/String;)V"),
                               jm);
        JNI_CHECK();
        if (override) {
            jmethodID set_override =
                (*env)->GetMethodID(env, connc, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
            jstring ok = (*env)->NewStringUTF(env, "X-HTTP-Method-Override"), ov = (*env)->NewStringUTF(env, override);
            (*env)->CallVoidMethod(env, conn, set_override, ok, ov);
            JNI_CHECK();
        }
    }
    {
        int tmo = net_fast ? 1200 : TIMEOUT;
        (*env)->CallVoidMethod(env, conn, (*env)->GetMethodID(env, connc, "setConnectTimeout", "(I)V"), tmo);
        JNI_CHECK();
        (*env)->CallVoidMethod(env, conn, (*env)->GetMethodID(env, connc, "setReadTimeout", "(I)V"), tmo);
        JNI_CHECK();
    }
    (*env)->CallVoidMethod(env, conn, (*env)->GetMethodID(env, connc, "setUseCaches", "(Z)V"), JNI_FALSE);
    JNI_CHECK();
    {
        jmethodID set_header =
            (*env)->GetMethodID(env, connc, "setRequestProperty", "(Ljava/lang/String;Ljava/lang/String;)V");
        jstring k = (*env)->NewStringUTF(env, "Content-Type"), v = (*env)->NewStringUTF(env, "application/json");
        (*env)->CallVoidMethod(env, conn, set_header, k, v);
        JNI_CHECK();
        if (header && value) {
            k = (*env)->NewStringUTF(env, header);
            v = (*env)->NewStringUTF(env, value);
            (*env)->CallVoidMethod(env, conn, set_header, k, v);
            JNI_CHECK();
        }
    }
    if (body && *body) {
        jobject os = NULL;
        jbyteArray data;
        jsize len = (jsize)strlen(body);
        (*env)->CallVoidMethod(env, conn, (*env)->GetMethodID(env, connc, "setDoOutput", "(Z)V"), JNI_TRUE);
        JNI_CHECK();
        (*env)->CallVoidMethod(env, conn, (*env)->GetMethodID(env, connc, "setFixedLengthStreamingMode", "(I)V"), len);
        JNI_CHECK();
        os = (*env)->CallObjectMethod(env, conn,
                                      (*env)->GetMethodID(env, connc, "getOutputStream", "()Ljava/io/OutputStream;"));
        JNI_CHECK();
        if (!os)
            goto done;
        data = (*env)->NewByteArray(env, len);
        JNI_CHECK();
        (*env)->SetByteArrayRegion(env, data, 0, len, (const jbyte *)body);
        (*env)->CallVoidMethod(env, os, (*env)->GetMethodID(env, (*env)->GetObjectClass(env, os), "write", "([B)V"),
                               data);
        JNI_CHECK();
        (*env)->CallVoidMethod(env, os, (*env)->GetMethodID(env, (*env)->GetObjectClass(env, os), "close", "()V"));
        JNI_CHECK();
    }
    code = (int)(*env)->CallIntMethod(env, conn, (*env)->GetMethodID(env, connc, "getResponseCode", "()I"));
    JNI_CHECK();
    if (etag && etag_cap) {
        jstring key = (*env)->NewStringUTF(env, "ETag");
        JNI_CHECK();
        jstring val = (jstring)(*env)->CallObjectMethod(
            env, conn, (*env)->GetMethodID(env, connc, "getHeaderField", "(Ljava/lang/String;)Ljava/lang/String;"),
            key);
        JNI_CHECK();
        if (val) {
            const char *s = (*env)->GetStringUTFChars(env, val, NULL);
            if (s) {
                snprintf(etag, etag_cap, "%s", s);
                (*env)->ReleaseStringUTFChars(env, val, s);
            }
        }
    }
    stream = (*env)->CallObjectMethod(
        env, conn,
        (*env)->GetMethodID(env, connc, code >= 400 ? "getErrorStream" : "getInputStream", "()Ljava/io/InputStream;"));
    JNI_CHECK();
    if (!stream)
        goto closeconn;
    streamc = (*env)->GetObjectClass(env, stream);
    JNI_CHECK();
    buf = (*env)->NewByteArray(env, 2048);
    JNI_CHECK();
    for (;;) {
        jint n = (*env)->CallIntMethod(env, stream, (*env)->GetMethodID(env, streamc, "read", "([B)I"), buf);
        JNI_CHECK();
        if (n <= 0)
            break;
        if (out && cap && total + (size_t)n < cap) {
            (*env)->GetByteArrayRegion(env, buf, 0, n, (jbyte *)(out + total));
            total += (size_t)n;
            out[total] = '\0';
        }
    }
    (*env)->CallVoidMethod(env, stream, (*env)->GetMethodID(env, streamc, "close", "()V"));
    JNI_CHECK();
closeconn:
    if (conn) {
        (*env)->CallVoidMethod(env, conn, (*env)->GetMethodID(env, connc, "disconnect", "()V"));
        (*env)->ExceptionClear(env);
    }
    ok = 1;
done:
    if ((*env)->ExceptionCheck(env)) {
        jthrowable ex = (*env)->ExceptionOccurred(env);
        (*env)->ExceptionClear(env);
        if (net_log_ok() && ex) {
            jclass tcls = (*env)->GetObjectClass(env, ex);
            jmethodID gm = tcls ? (*env)->GetMethodID(env, tcls, "getMessage", "()Ljava/lang/String;") : NULL;
            if (gm) {
                jstring jmsg = (jstring)(*env)->CallObjectMethod(env, ex, gm);
                if (jmsg && !(*env)->ExceptionCheck(env)) {
                    const char *s = (*env)->GetStringUTFChars(env, jmsg, NULL);
                    if (s) {
                        LOGERR("http %s: %s", method, s);
                        (*env)->ReleaseStringUTFChars(env, jmsg, s);
                    }
                }
            }
        }
        (*env)->ExceptionClear(env);
    }
    (*env)->PopLocalFrame(env, NULL);
    if (attached)
        (*net.vm)->DetachCurrentThread(net.vm);
    return ok ? code : 0;
#undef JNI_CHECK
}
