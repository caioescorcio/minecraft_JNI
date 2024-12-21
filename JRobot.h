#pragma once

#include <jni.h>

class JRobot final {

public:

	JRobot(JNIEnv* p_env) : p_env(p_env) {
		Init();
	}

	bool Init() {
		class_ptr = p_env->FindClass("java/awt/Robot");
		if (class_ptr == nullptr) {
			printf("[-] Falha ao achar ponteiro para a classe %s\n", nome);
			return false;
		}
		
		jmethodID constructor_id = p_env->GetMethodID(class_ptr, "<init>", "()V");

		class_instance = p_env->NewObject(class_ptr, constructor_id);
		if (class_ptr == nullptr) {
			printf("[-] Falha ao achar instancia para a classe %s\n", nome);
			return false;
		}
	}

	//
	// Funcoes a serem chamadas
	//

	void mousePress(jint buttons) {
		static jmethodID method_id = p_env->GetMethodID(class_ptr, "mousePress", "(I)V");
		p_env->CallVoidMethodA(class_instance, method_id, (jvalue*)&buttons);
	}
	void mouseRelease(jint buttons) {
		static jmethodID method_id = p_env->GetMethodID(class_ptr, "mouseRelease", "(I)V");
		p_env->CallVoidMethodA(class_instance, method_id, (jvalue*)&buttons);
	}
	void delay(jint ms) {
		static jmethodID method_id = p_env->GetMethodID(class_ptr, "delay", "(I)V");
		p_env->CallVoidMethodA(class_instance, method_id, (jvalue*)&ms);
	}


private:
	jclass class_ptr;
	jobject class_instance;
	JNIEnv* p_env;
	const char nome[7] = { "JRobot" };
};
