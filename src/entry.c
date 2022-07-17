#include <pthread.h>
#include <dlfcn.h>
#include <pthread.h>

#include "cheat.h"
#include "jni.h"

typedef void (*JVM_MonitorNotify)(JNIEnv *env, jobject obj);

typedef jint (JNICALL * Jvm) (JavaVM**, jsize, jsize*);

JVM_MonitorNotify MonitorNotify = NULL;

void* hook(void* args);
#define UNUSED(x) (void)(x)

__attribute__((constructor)) static void entry() {
	   pthread_t ptid;
	   pthread_create(&ptid, NULL, &hook, NULL);
	   pthread_detach(ptid);
}

void * handle_jvm = NULL;

void* hook(void* args) {
	UNUSED(args);
	int n_vms = 1;
	JavaVM *vm = NULL;
	JNIEnv *env = NULL;

	handle_jvm = dlopen("libjvm.so", RTLD_LAZY);
	Jvm pJvm = NULL;
	*(void **) (&pJvm) = dlsym(handle_jvm, "JNI_GetCreatedJavaVMs");

	pJvm(&vm, n_vms, &n_vms);
	(*vm)->AttachCurrentThread(vm, (void*) &env, NULL);

	cheat(env);
	return NULL;
}
