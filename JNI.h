#pragma once
#include <memory>
#include <Windows.h>
#include <jni.h>
#include "JRobot.h"

class JNI final{

public:
	// Instancia do JRobot
	std::unique_ptr<JRobot> p_jrobot;

	// Construtor
	JNI() {

		// Inicializando os ponteiros basicos
		{
			jint result = JNI_GetCreatedJavaVMs(&p_jvm, 1, nullptr);
			if (result != 0) {
				printf("[-] Falha ao inicializar ponteiro para JVM (p_jvm)");
				MessageBoxA(0, "ERROR", "Check Console", MB_ICONASTERISK);
			}

			p_jvm->AttachCurrentThread((void**)&p_env, nullptr);
		}

		// Inicializando classes do jogo
		{
			p_jrobot = std::make_unique<JRobot>(p_env);
		}
		
		is_init = true;

	}

	// Destruidor
	~JNI() {
		p_jvm->DetachCurrentThread();
		is_init = false;
	}

	bool GetInit() {
		return is_init;
	}

private:
	JavaVM* p_jvm;
	JNIEnv* p_env;

	bool is_init{ false };
};


inline std::unique_ptr<JNI> p_jni;
