
### Primeiro passo:
Ao configurar o projeto o primeiro trecho de codigo a ser escrito e o da `main.cpp`:

```cpp
#include <Windows.h>

bool __stdcall DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	if(reason == DLL_PROCESS_ATTACH) {

	}
	else if (reason == DLL_PROCESS_DETACH) {

	}
	
	return true;
}
```
Nele, temos o include da lib `Windows.h`, que contem as estruturas necessarias para a criacao da .dll:

<br>

**`instance` (HINSTANCE)**  
- Eh um **handle** que representa a instancia do modulo (neste caso, a DLL) na memoria.
- Ele eh usado para identificar o modulo atual na aplicacao e pode ser utilizado para carregar recursos, localizar informacoes ou gerenciar a DLL.
- Quando uma DLL eh carregada, o sistema operacional fornece esse identificador exclusivo para a instancia da DLL, como se fosse um Singleton.

**`reason` (DWORD)**  
- Indica o motivo pelo qual a funcao `DllMain` foi chamada. Pode ter um dos seguintes valores:
  - **`DLL_PROCESS_ATTACH` (1)**: A DLL esta sendo carregada no contexto de um processo pela primeira vez.
  - **`DLL_PROCESS_DETACH` (0)**: A DLL esta sendo descarregada do contexto de um processo.
  - **`DLL_THREAD_ATTACH` (2)**: Um novo thread esta sendo criado no processo que ja esta utilizando a DLL.
  - **`DLL_THREAD_DETACH` (3)**: Um thread existente, que esta usando a DLL, esta sendo encerrado.
- No codigo, a priori, somente os casos `DLL_PROCESS_ATTACH` e `DLL_PROCESS_DETACH` estao sendo tratados.

**`reserved` (LPVOID)**  
- Eh um ponteiro generico passado pelo sistema operacional.
- Ele pode ter dois valores principais:
  - Se a `reason` for **`DLL_PROCESS_ATTACH`**, o ponteiro sera `NULL` se a DLL foi carregada explicitamente com `LoadLibrary` ou nao sera `NULL` se a DLL foi carregada implicitamente.
  - Para outros valores de `reason`, geralmente o valor nao eh relevante.

**`stdcall`**  
- Eh uma **convencao de chamada** usada na funcao `DllMain`.
- Define como os argumentos sao passados para a funcao e quem eh responsavel por limpar a pilha apos a chamada:
  - Os argumentos sao empilhados da direita para a esquerda.
  - A funcao chamada (`DllMain`, neste caso) eh responsavel por limpar a pilha.
- Essa convencao eh amplamente utilizada no Windows, especialmente em APIs e DLLs, para garantir consistencia entre chamadas.

	- **`DLL_PROCESS_ATTACH` e `DLL_PROCESS_DETACH`** sao dois dos valores que podem ser atribuidos a `reason`:

	  - **`DLL_PROCESS_ATTACH`**
		- Ocorre quando a DLL eh carregada no processo pela primeira vez.
		- Normalmente usado para inicializacoes, como alocar recursos globais ou configurar dados necessarios para a operacao da DLL.

	  - **`DLL_PROCESS_DETACH`**
		- Ocorre quando a DLL eh descarregada de um processo.
		- Usado para limpar recursos alocados durante a inicializacao ou liberar memoria que nao sera mais utilizada.

Ou sejam quando o processo que usa a DLL a carrega, `DllMain` eh chamado com o valor `DLL_PROCESS_ATTACH`. Nesse momento, voce poderia inicializar variaveis globais ou alocar recursos. Quando o processo termina ou explicitamente descarrega a DLL, `DllMain` eh chamado novamente com `DLL_PROCESS_DETACH`, onde voce poderia liberar recursos ou executar tarefas de limpeza.

Em seguida, complementa-se o c�digo com algumas modifica��es:

```cpp

#include <thread>
#include <cstdio>
#include <Windows.h>
#include <jni.h>

void MainThread(HMODULE instance) {

	JavaVM* p_jvm{ nullptr };
	jint result = JNI_GetCreatedJavaVMs(&p_jvm, 1, nullptr);

	void* p_env{ nullptr }; 
	
	p_jvm->AttachCurrentThread(&p_env,nullptr);

	while (!GetAsyncKeyState(VK_END)) {
		printf("%d",GetAsyncKeyState(VK_DOWN));
	}
	FreeLibrary(instance);
}

bool __stdcall DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {

	static FILE* p_file{ nullptr };
	static std::thread main_thread;

	if(reason == DLL_PROCESS_ATTACH) {

		AllocConsole();
		freopen_s(&p_file, "CONOUT$", "w", stdout);
		main_thread = std::thread([instance] { MainThread(instance);});
		if (main_thread.joinable()) {
			main_thread.detach();
		}
	}
	else if (reason == DLL_PROCESS_DETACH) {

		fclose(p_file);
		FreeConsole();

	}
	
	return true;

```

Trecho a trecho, primeiramente as bibliotecas que foram instaladas no programa sao `thread`, `cstdio` e `jni`. Respectivamente elas contem as funcoes de uso de threads, de input e output e de Java Native Interface (que eh usado como um meio do Java usar libs de outras linguagens na JVM/Uso do sistema operacional).

Alem disso, na funcao MainThread, serao executadas as principais logicas do programa, que serao usadas em cada thread:

<br>

- `JavaVM* p_jvm{ nullptr };`: inicia um ponteiro para onde esta alocada a JavaVM
- `jint result = JNI_GetCreatedJavaVMs(&p_jvm, 1, nullptr);`: verifica a instancia da JNI criada e a armazena no `p_jvm`. `1` significa que somente uma instancia sera criada. Se `result != 0` significa que nenhuma JVM foi encontrada (erro)
- `void* p_env{ nullptr };`, inicia o ponteiro de ambiente, ele representara o ambiente JNI associado a thread atual
- `p_jvm->AttachCurrentThread(&p_env, nullptr);`, anexa a thread atual ao ambiente da JVM, recebendo o ponteiro de `env` para associa-lo ao JNI 
- `while (!GetAsyncKeyState(VK_END)) {}`, condicao de execucao enquanto a tecla END nao eh pressionada
- `FreeLibrary(instance);`, libera a DLL do processo, limpando os dados usados

- O `printf("%d",GetAsyncKeyState(VK_DOWN));` eh uma forma de print para verificar o estado do botao DOWM do teclado, para ver se o codigo funciona

Agora, ha um trecho de codigo realiza a configuracao de um console para saida padr�o e a criacao de uma thread principal quando a DLL eh carregada, alem de garantir a limpeza desses recursos ao descarregar a DLL:

- `static FILE* p_file{ nullptr };`, declara um ponteiro para um arquivo que sera usado para redirecionar a saida padr�o (`stdout`) para o console. A palavra-chave `static` garante que a variavel seja compartilhada por todas as instancias da DLL.
  
- `static std::thread main_thread;`, declara uma variavel estatica que representa a thread principal criada pela DLL. Tambem eh compartilhada globalmente.


- `AllocConsole();`, cria uma nova janela de console associada ao processo atual, permitindo entrada e sa�da padr�o (stdin, stdout e stderr).

- `freopen_s(&p_file, "CONOUT$", "w", stdout);`, redireciona o fluxo de saida padrao (`stdout`) para a nova janela de console.  
  - `CONOUT$` eh um dispositivo especial que representa a saida padrao no console.
  - O ponteiro `p_file` armazena a referencia ao fluxo, permitindo que seja fechado posteriormente.

- `main_thread = std::thread([instance] { MainThread(instance); });`, cria uma nova thread que executa a funcao `MainThread`, passando a variavel `instance` como argumento capturado. Isso permite que a execucao da logica principal ocorra de forma independente da thread principal do processo que carregou a DLL.

- `if (main_thread.joinable()) { main_thread.detach(); }`, verifica se a thread pode ser "juntada" (`joinable()`), ou seja, se ainda nao foi descartada ou unida. Caso seja "juntavel", a thread eh desanexada (`detach()`), permitindo que ela continue sua execucao de forma independente sem bloquear a thread principal.



- `fclose(p_file);`, fecha o fluxo de saida padrao que foi redirecionado ao console.

- `FreeConsole();`, libera o console associado ao processo, encerrando sua utiliza��o.


Em resumo, esse codigo ao ser executado como DLL permite ao usuario visualizar pelo terminal certos acontecimentos do app usando a DLL.
Para compilar no Visual Studio, use Ctrl+B

### Uso

Para usar o codigo, eh necessario usar o software Process Hacker, para injertar a DLL:

- Abra o Minecraft e o Process Hacker, procure pelo java.exe
- Clique com o botao direito do mouse e, em Miscellaneous, clique em Inject DLL
- Selecione a DLL buildada ao rodar o codigo principal