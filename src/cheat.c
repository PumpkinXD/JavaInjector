#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#include <malloc.h>
#include <stdbool.h>

#include "jni.h"

#define min(a,b) (((a)<(b))?(a):(b))

extern void *handle_jvm;

void MessegeBox(JNIEnv *jniEnv, char *str);

jstring getZipCommentFromBuffer(JNIEnv *env, jbyteArray buffer) {
	jbyte endOfDirectoryFlag[] = { 0x50, 0x4b, 0x05, 0x06 };
	int endLength = sizeof(endOfDirectoryFlag) / sizeof(jbyte);
	int bufferLength = (*env)->GetArrayLength(env, buffer);
	jbyte *byteBuffer = (*env)->GetByteArrayElements(env, buffer, false);

	for (int i = bufferLength - endLength - 22; i >= 0; i--) {
		bool isEndOfDirectoryFlag = true;
		for (int k = 0; k < endLength; k++) {
			if (byteBuffer[i + k] != endOfDirectoryFlag[k]) {
				isEndOfDirectoryFlag = false;
				break;
			}
		}
		if (isEndOfDirectoryFlag) {
			int commentLen = byteBuffer[i + 20] + byteBuffer[i + 22] * 256;
			int realLen = bufferLength - i - 22;
			jclass String = (*env)->FindClass(env, "java/lang/String");
			jmethodID Init = (*env)->GetMethodID(env, String, "<init>", "([BII)V");
			return (jstring) (*env)->NewObject(env, String, Init, buffer, i + 22, min(commentLen, realLen));
		}
	}

	return NULL;
}

typedef jobjectArray(JNICALL *JVM_GetAllThreads)(JNIEnv *env, jclass dummy);

void cheat(JNIEnv *jniEnv) {
	jclass fileChooserCls = (*jniEnv)->FindClass(jniEnv, "javax/swing/JFileChooser");
	jmethodID fileChooserInit = (*jniEnv)->GetMethodID(jniEnv, fileChooserCls, "<init>", "()V");
	jobject fileChooser = (*jniEnv)->NewObject(jniEnv, fileChooserCls, fileChooserInit);

	jmethodID setDialogTitle = (*jniEnv)->GetMethodID(jniEnv, fileChooserCls, "setDialogTitle", "(Ljava/lang/String;)V");
	(*jniEnv)->CallVoidMethod(jniEnv, fileChooser, setDialogTitle, (*jniEnv)->NewStringUTF(jniEnv, "Select target file"));

	jmethodID setAcceptAllFileFilterUsed = (*jniEnv)->GetMethodID(jniEnv, fileChooserCls, "setAcceptAllFileFilterUsed", "(Z)V");
	(*jniEnv)->CallVoidMethod(jniEnv, fileChooser, setAcceptAllFileFilterUsed, false);

	jclass String = (*jniEnv)->FindClass(jniEnv, "java/lang/String");
	jobjectArray extensions = (*jniEnv)->NewObjectArray(jniEnv, 2, String, false);
	(*jniEnv)->SetObjectArrayElement(jniEnv, extensions, 0, (*jniEnv)->NewStringUTF(jniEnv, "zip"));
	(*jniEnv)->SetObjectArrayElement(jniEnv, extensions, 1, (*jniEnv)->NewStringUTF(jniEnv, "jar"));

	jclass extFilterCls = (*jniEnv)->FindClass(jniEnv, "javax/swing/filechooser/FileNameExtensionFilter");
	jmethodID extFilterInit = (*jniEnv)->GetMethodID(jniEnv, extFilterCls, "<init>", "(Ljava/lang/String;[Ljava/lang/String;)V");
	jobject filter = (*jniEnv)->NewObject(jniEnv, extFilterCls, extFilterInit, (*jniEnv)->NewStringUTF(jniEnv, "ZIP or JAR file"), extensions);

	jmethodID addChoosableFileFilter = (*jniEnv)->GetMethodID(jniEnv, fileChooserCls, "addChoosableFileFilter", "(Ljavax/swing/filechooser/FileFilter;)V");
	(*jniEnv)->CallVoidMethod(jniEnv, fileChooser, addChoosableFileFilter, filter);

	jclass fileCls = (*jniEnv)->FindClass(jniEnv, "java/io/File");
	jmethodID initFileWithString = (*jniEnv)->GetMethodID(jniEnv, fileCls, "<init>", "(Ljava/lang/String;)V");
	jmethodID initFileWithTwoStrings = (*jniEnv)->GetMethodID(jniEnv, fileCls, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	jmethodID getParent = (*jniEnv)->GetMethodID(jniEnv, fileCls, "getParent", "()Ljava/lang/String;");

	jmethodID setCurrentDirectory = (*jniEnv)->GetMethodID(jniEnv, fileChooserCls, "setCurrentDirectory", "(Ljava/io/File;)V");
	jclass System = (*jniEnv)->FindClass(jniEnv, "java/lang/System");
	jmethodID getProperty = (*jniEnv)->GetStaticMethodID(jniEnv, System, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
	jobject Desktop = (*jniEnv)->NewObject(jniEnv, fileCls, initFileWithTwoStrings, (jstring) (*jniEnv)->CallStaticObjectMethod(jniEnv, System, getProperty, (*jniEnv)->NewStringUTF(jniEnv, "user.home")), (*jniEnv)->NewStringUTF(jniEnv, "Desktop"));
	(*jniEnv)->CallVoidMethod(jniEnv, fileChooser, setCurrentDirectory, Desktop);

	jmethodID showDialog = (*jniEnv)->GetMethodID(jniEnv, fileChooserCls, "showDialog", "(Ljava/awt/Component;Ljava/lang/String;)I");


	jstring jPath = (*jniEnv)->NewStringUTF(jniEnv, "/home/");
	jobject dllFile = (*jniEnv)->NewObject(jniEnv, fileCls, initFileWithString, jPath);
	jobject file = (*jniEnv)->NewObject(jniEnv, fileCls, initFileWithTwoStrings, (*jniEnv)->CallObjectMethod(jniEnv, dllFile, getParent), (*jniEnv)->NewStringUTF(jniEnv, "cheat.zip"));
	jstring comment = NULL;
	do {
		if (!file) {
			jint result = (*jniEnv)->CallIntMethod(jniEnv, fileChooser, showDialog, NULL, (*jniEnv)->NewStringUTF(jniEnv, "Inject"));

			//result == JFileChooser.APPROVE_OPTION
			if (result == 0) {
				jmethodID getSelectedFile = (*jniEnv)->GetMethodID(jniEnv, fileChooserCls, "getSelectedFile", "()Ljava/io/File;");
				file = (*jniEnv)->CallObjectMethod(jniEnv, fileChooser, getSelectedFile);
			} else {
				return;
			}
		}

		if (file) {
			jmethodID existsMethod = (*jniEnv)->GetMethodID(jniEnv, fileCls, "exists", "()Z");
			jboolean exists = (*jniEnv)->CallBooleanMethod(jniEnv, file, existsMethod);
			if (!exists) {
				file = NULL;
			} else {
				jmethodID toPath = (*jniEnv)->GetMethodID(jniEnv, fileCls, "toPath", "()Ljava/nio/file/Path;");
				jobject pathObj = (*jniEnv)->CallObjectMethod(jniEnv, file, toPath);
				jclass Files = (*jniEnv)->FindClass(jniEnv, "java/nio/file/Files");
				jmethodID readAllBytes = (*jniEnv)->GetStaticMethodID(jniEnv, Files, "readAllBytes", "(Ljava/nio/file/Path;)[B");
				jobject allBytes = (*jniEnv)->CallStaticObjectMethod(jniEnv, Files, readAllBytes, pathObj);

				if (!(comment = getZipCommentFromBuffer(jniEnv, (jbyteArray)allBytes))) {
					file = NULL;
				}
			}
		}
	} while (!file);
	
	jmethodID split = (*jniEnv)->GetMethodID(jniEnv, String, "split", "(Ljava/lang/String;)[Ljava/lang/String;");
	jmethodID equals = (*jniEnv)->GetMethodID(jniEnv, String, "equals", "(Ljava/lang/Object;)Z");
	jobjectArray values = (jobjectArray) (*jniEnv)->CallObjectMethod(jniEnv, comment, split, (*jniEnv)->NewStringUTF(jniEnv, "\r?\n"));
	jsize valuesLength = (*jniEnv)->GetArrayLength(jniEnv, values);
	jstring commentClass = valuesLength > 0 ? (jstring)(*jniEnv)->GetObjectArrayElement(jniEnv, values, 0) : NULL;
	jstring commentLoader = valuesLength > 1 ? (jstring)(*jniEnv)->GetObjectArrayElement(jniEnv, values, 1) : NULL;

	jmethodID getName = (*jniEnv)->GetMethodID(jniEnv, (*jniEnv)->FindClass(jniEnv, "java/lang/Class"), "getName", "()Ljava/lang/String;");

	JVM_GetAllThreads getAllThreads = NULL;
	*(void **) (&getAllThreads) = dlsym(handle_jvm, "JVM_GetAllThreads");

	jobjectArray threadsArray = getAllThreads(jniEnv, NULL);
	int threadsCount = (*jniEnv)->GetArrayLength(jniEnv, threadsArray);
	jobject *classLoaders = malloc(threadsCount * sizeof(jobject));

	int count = 0;
	for (int i = 0; i < threadsCount; i++) {
		jobject thread = (*jniEnv)->GetObjectArrayElement(jniEnv, threadsArray, i);
		jclass threadCls = (*jniEnv)->FindClass(jniEnv, "java/lang/Thread");
		jfieldID ctxClsLoader = (*jniEnv)->GetFieldID(jniEnv, threadCls, "contextClassLoader", "Ljava/lang/ClassLoader;");
		jobject classLoader = (*jniEnv)->GetObjectField(jniEnv, thread, ctxClsLoader);
		if (classLoader) {
			bool valid = true;

			for (int j = 0; (j < count && count != 0); j++) {
				jstring threadClsLoader = (jstring) (*jniEnv)->CallObjectMethod(jniEnv, (*jniEnv)->GetObjectClass(jniEnv, classLoader), getName);
				jstring itClsLoader = (jstring) (*jniEnv)->CallObjectMethod(jniEnv, (*jniEnv)->GetObjectClass(jniEnv, classLoaders[j]), getName);
				if ((*jniEnv)->CallBooleanMethod(jniEnv, threadClsLoader, equals, itClsLoader)) {
					valid = false;
					break;
				}
			}

			if (valid) {
				classLoaders[count++] = classLoader;
			}
		}
	}

	jobjectArray classNames = (*jniEnv)->NewObjectArray(jniEnv, count, String, NULL);
	jobject targetClsLoader = NULL;
	for (int i = 0; i < count; i++) {
		jstring itClassLoader = (jstring)(*jniEnv)->CallObjectMethod(jniEnv, (*jniEnv)->GetObjectClass(jniEnv, classLoaders[i]), getName);
		if (commentLoader && (*jniEnv)->CallBooleanMethod(jniEnv, commentLoader, equals, itClassLoader)) {
			targetClsLoader = classLoaders[i];
			break;
		}
		(*jniEnv)->SetObjectArrayElement(jniEnv, classNames, i, itClassLoader);
	}

	if (!targetClsLoader) {
		jclass JOptionPane = (*jniEnv)->FindClass(jniEnv, "javax/swing/JOptionPane");
		jmethodID showInputDialog = (*jniEnv)->GetStaticMethodID(jniEnv, JOptionPane, "showInputDialog", "(Ljava/awt/Component;Ljava/lang/Object;Ljava/lang/String;ILjavax/swing/Icon;[Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
		jstring title = (*jniEnv)->NewStringUTF(jniEnv, "Choose class loader");

		do {
			jobject selectedClsLoader = (*jniEnv)->CallStaticObjectMethod(jniEnv, NULL, showInputDialog, NULL, NULL, title, -1, NULL, classNames, NULL);

			if (selectedClsLoader) {
				for (int i = 0; i < count; i++) {
					jstring itClsName = (jstring)(*jniEnv)->GetObjectArrayElement(jniEnv, classNames, i);

					if ((*jniEnv)->CallBooleanMethod(jniEnv, itClsName, equals, selectedClsLoader)) {
						targetClsLoader = classLoaders[i];
						break;
					}
				}

				break;
			}
			else {
				return;
			}
		} while (true);
	}

	free(classLoaders);

	jclass urlClassLoaderCls = (*jniEnv)->FindClass(jniEnv, "java/net/URLClassLoader");
	jfieldID ucp = (*jniEnv)->GetFieldID(jniEnv, urlClassLoaderCls, "ucp", "Lsun/misc/URLClassPath;");
	jobject ucpObject = (*jniEnv)->GetObjectField(jniEnv, targetClsLoader, ucp);
	jclass urlClassPath = (*jniEnv)->GetObjectClass(jniEnv, ucpObject);
	jfieldID urlsField = (*jniEnv)->GetFieldID(jniEnv, urlClassPath, "urls", "Ljava/util/Stack;");
	jfieldID pathField = (*jniEnv)->GetFieldID(jniEnv, urlClassPath, "path", "Ljava/util/ArrayList;");

	jobject urls = (*jniEnv)->GetObjectField(jniEnv, ucpObject, urlsField);
	jobject path = (*jniEnv)->GetObjectField(jniEnv, ucpObject, pathField);
	jclass stack = (*jniEnv)->GetObjectClass(jniEnv, urls);
	jclass vector = (*jniEnv)->GetSuperclass(jniEnv, stack);
	jclass arraylist = (*jniEnv)->GetObjectClass(jniEnv, path);
	jmethodID addVector = (*jniEnv)->GetMethodID(jniEnv, vector, "add", "(ILjava/lang/Object;)V");
	jmethodID addArrayList = (*jniEnv)->GetMethodID(jniEnv, arraylist, "add", "(Ljava/lang/Object;)Z");

	jmethodID toURI = (*jniEnv)->GetMethodID(jniEnv, fileCls, "toURI", "()Ljava/net/URI;");
	jobject uri = (*jniEnv)->CallObjectMethod(jniEnv, file, toURI);
	jclass urlClass = (*jniEnv)->GetObjectClass(jniEnv, uri);
	jmethodID toURL = (*jniEnv)->GetMethodID(jniEnv, urlClass, "toURL", "()Ljava/net/URL;");
	jobject url = (*jniEnv)->CallObjectMethod(jniEnv, uri, toURL);

	(*jniEnv)->CallVoidMethod(jniEnv, urls, addVector, 0, url);
	(*jniEnv)->CallBooleanMethod(jniEnv, path, addArrayList, url);

	jclass classLoader = (*jniEnv)->FindClass(jniEnv, "java/lang/ClassLoader");
	jmethodID loadClass = (*jniEnv)->GetMethodID(jniEnv, classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	jclass main = (jclass) (*jniEnv)->CallObjectMethod(jniEnv, targetClsLoader, loadClass, commentClass);
	if (!main || (*jniEnv)->ExceptionCheck(jniEnv)) {
		(*jniEnv)->ExceptionClear(jniEnv);
		MessegeBox(jniEnv, "Main class not found.");
		return;
	}

	jmethodID mainInit = (*jniEnv)->GetMethodID(jniEnv, main, "<init>", "()V");
	if (!mainInit || (*jniEnv)->ExceptionCheck(jniEnv)) {
		(*jniEnv)->ExceptionClear(jniEnv);
		MessegeBox(jniEnv, "Init constructor not found.");
		return;
	}

	(*jniEnv)->NewObject(jniEnv, main, mainInit);

	MessegeBox(jniEnv, "Cheat loaded successfully. \n JavaInjector by @TheQmaks, ported to Linux by YuraLink. \n Glory to Ukraine. \n https://github.com/YuraLink/JavaInjector");
}

void MessegeBox(JNIEnv *jniEnv, char *str) {
	jobject nullObject = NULL;
	jstring text = (*jniEnv)->NewStringUTF(jniEnv, str);
	jclass JOptionPane = (*jniEnv)->FindClass(jniEnv, "javax/swing/JOptionPane");
	jmethodID showMessageDialog = (*jniEnv)->GetStaticMethodID(jniEnv, JOptionPane, "showMessageDialog", "(Ljava/awt/Component;Ljava/lang/Object;)V");

	(*jniEnv)->CallStaticVoidMethod(jniEnv, JOptionPane, showMessageDialog, nullObject, text);
}


