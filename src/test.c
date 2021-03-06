/*
 * Здесь можно наблюдать пример
 * структуры плагинов для сервера,
 * использовать только для ознакомления
 * с PluginAPI.
*/

/*
 * Хедеры сервера. Файл core.h должен быть
 * всегда подключен в главном файле кода плагина.
*/
#include <core.h>
#include <str.h>
#include <log.h>
#include <client.h>
#include <block.h>
#include <event.h>
#include <command.h>
#include <world.h>
#include <plugin.h>

// Хедеры плагина
#include "test.h"

/*
 * Дополнительная информация об
 * энумах, используемых при объявлении
 * структур BlockDef может быть найдена
 * в файле block.h.
*/

static BlockDef myBlock = {
	"My test block", FALLBACK_BLOCK_ID, 0,
	{{
		BDSOL_SOLID,
		255,
		8, 14, 80,
		false,
		BDSND_STONE,
		false,
		8,
		BDDRW_OPAQUE,
		0, 0, 0, 0
	}}
};

static BlockDef myExtendedBlock = {
	"My extended test block", FALLBACK_BLOCK_ID_EXT, BDF_EXTENDED,
	{{
		BDSOL_SWIM,
		10,
		17, 37, 53, 51, 35, 49,
		false,
		BDSND_GRASS,
		false,
		10, 10, 10,
		13, 13, 13,
		BDDRW_OPAQUE,
		127, 0, 0, 0
	}}
};

static BlockDef *myDynBlock = NULL;
static cs_bool enabled = false;

/*
 * В функцию эвента передаётся первый параметр,
 * который был передан функции вызова эвента,
 * если параметр был один, то передаётся только он,
 * без поинтера на пачало параметров функции.
 * Мне показалось удобным кастование аргументов
 * в структуру для дальнейшего взаимодействия с ними
 * более удобного способа я не придумал.
*/
static void onmesgfunc(void *param) {
  if(enabled)
	((onMessage *)param)->type = MESSAGE_TYPE_ANNOUNCE;
}

void onworldload(World *world) {
	/*
	 * С помощью дополнения BlockDefinitions
	 * можно регистрировать новые блоки для
	 * клиентов. Фунцкия Block_Define принимает
	 * указатель на структуру BlockDef, в
	 * которую записана информация о блоке,
	 * который необходимо зарегистрировать.
	 * Структура BlockDef содержит 4 поля:
	 * флаги, ID блока, его название и
	 * параметры. Если нужно создать
	 * расширенный блок, то в структуре
	 * следует установить флаг BDF_EXTENDED.
	 * P.S. Если имеется непреодалимое желание
	 * модифицировать имеющийся блок, то после
	 * внесения изменений в его параметры
	 * необходимо установить флаг BDF_UPDATED
	 * (block.flags |= BDF_UPDATED), а затем
	 * вызывать функцию Block_UpdateDefinition.
	*/
	Block_Define(world, BLOCK_ID, &myBlock);
	Block_Define(world, BLOCK_ID_EXT, &myExtendedBlock);

	/*
	 * Структура BlockDef также может
	 * находиться в динамической памяти, для этого
	 * её нужно создать через функцию Block_New, где
	 * аргументами являются имя блока и его флаги
	 * соответственно. После завершения выделения памяти
	 * функция вернёт BlockDef, поинтер на BlockDef. При
	 * динамической аллокации структуры происходит также
	 * КОПИРОВАНИЕ имени блока, соответственно, переданный в
	 * функцию cs_str не обязан указывать постоянно на
	 * имя блока. При динамической аллокации структуре блока
	 * автоматически устанавливается флаг BDF_DYNALLOCED, для
	 * чего он нужен описано в теле Plugin_Unload.
	*/
	Block_Define(world, BLOCK_ID_DYN, myDynBlock);
}

/*
 * При вызове команды из консоли сервера аргумент "caller" будет NULL.
 * Также стоит заметить, что "args" тоже будет NULL при отсутствии аргументов.
*/
COMMAND_FUNC(PlugTest) {
  COMMAND_PRINT("This command registred by testplugin." DLIB_EXT);
}

COMMAND_FUNC(Atoggle) {
	// Макрос проверяет была ли запущена команда администратором
  enabled ^= 1;
	COMMAND_PRINTF("Announce chat %s", enabled ? "&aenabled" : "&cdisabled");
}

COMMAND_FUNC(Announce) {
	Client_Chat(CLIENT_BROADCAST, MESSAGE_TYPE_ANNOUNCE, ccdata->args);
	COMMAND_PRINT("Announcement sent.");
}

/*
 * Исходя из названия команды,
 * даже самому тупенькому будет понятно:
 * Эта команда уничтожает сама себя после
 * исполнения, то есть, когда она будет
 * вызвана однажды - её нельзя будет вызвать
 * вновь, вплоть до перезапуска сервера.
*/
COMMAND_FUNC(SelfDestroy) {
	Command_Unregister(ccdata->command);
	COMMAND_PRINT("This command can't be called anymore");
}

/*
 * Если в начало обработчика команды
 * сунуть макрос Command_OnlyForClient,
 * то команда будет выполнена только в том
 * случае, если вызвана она была игроком,
 * в противном случае в консоль выводится
 * сообщение о том, что команду может вызвать
 * только игрок.
*/
COMMAND_FUNC(ClientOnly) {
	COMMAND_PRINTF("Client-only command called by %s", Client_GetName(ccdata->caller));
}

/*
 * Вызов этого макроса обязателен, он устанавливает
 * не только версию плагина, но и версию используемого
 * API сервера, которая используется при загрузке плагина.
*/
Plugin_SetVersion(1);

cs_bool Plugin_Load(void) { // Основная функция, вызывается после подгрузки плагина.
	Event_RegisterVoid(EVT_ONMESSAGE, onmesgfunc); // Регистрация обработчика эвента.
	Event_RegisterVoid(EVT_ONWORLDADDED, onworldload);
	COMMAND_ADD(PlugTest, CMDF_NONE, "Test command"); // Регистрация обработчика команд.
	COMMAND_ADD(Atoggle, CMDF_OP, "Test command");
	COMMAND_ADD(Announce, CMDF_OP, "Test command");
	COMMAND_ADD(SelfDestroy, CMDF_NONE, "Test command");
	COMMAND_ADD(ClientOnly, CMDF_CLIENT, "Test command");
	// Любая Log-функция принимает vararg и работает также, как и printf.
	Log_Info("Test plugin loaded"); // Отправка в консоль INFO сообщения.
	Log_Debug("It's a debug message");
	Log_Warn("It's a warning message");

	myDynBlock = Block_New("My dynamically allocated block", 0);

	/*
	 * Эта функция должна вызываться как после изменений
	 * в структурах уже зарегистрированных блоков, так и
	 * после регистрации новых блоков. Она рассылает
	 * всем клиентам пакеты удаляющие или добавляющие
	 * новые блоки. Функцию нужно вызывать даже, если
	 * на сервере нет игроков, так как она производит
	 * манипуляции с полем "flags".
	*/
	Block_UpdateDefinition(&myBlock);
	Block_UpdateDefinition(&myExtendedBlock);
	Block_UpdateDefinition(myDynBlock);

	/*
	 * Если функция вернула true, значит
	 * плагин удалось успешно загрузить.
	 * Если функция вернёт false, сервер
	 * выгрузит динамическую библиотеку
	 * плагина из памяти и не будет
	 * больше на неё ссылаться.
	*/
  return true;
}

cs_bool Plugin_Unload(cs_bool force) {
	(void)force;
	/*
	 * Вызов Unregister функций внутри
	 * функции плагина Unload обязателен,
	 * так как он говорит серверу, чтобы тот
	 * не ссылался больше на эти участки памяти,
	 * ибо в скором времени они станут недоступны
	 * и обращение к ним приведёт к падению, а нам
	 * оно не нужно.
	*/
	Event_Unregister(EVT_ONMESSAGE, (void *)onmesgfunc);
	COMMAND_REMOVE(PlugTest);
	COMMAND_REMOVE(Atoggle);
	COMMAND_REMOVE(Announce);
	COMMAND_REMOVE(SelfDestroy);
	COMMAND_REMOVE(ClientOnly);

	/*
	 * Функция Block_UndefineGlobal ТОЛЬКО УСТАНАВЛИВАЕТ
	 * ФЛАГИ, она не производит больше никаких
	 * манипуляций. Тем не менее, блоки, имеющие
	 * флаг BDF_UNDEFINED не будут отправлены игроку
	 * даже если после Block_UndefineGlobal не была вызвана
	 * функция Block_UpdateDefinition.
	*/
	Block_UndefineGlobal(&myBlock);
	Block_UndefineGlobal(&myExtendedBlock);
	Block_UndefineGlobal(myDynBlock);

	/*
	 * Здесь вызов Block_UpdateDefinition нужен, чтобы
	 * разослать игрокам пакет RemoveBlockDefinition,
	 * убрать блок из массива, а также чтобы высвободить
	 * место, выделенное под поле "name" и саму структуру,
	 * если в ней установлен флаг BDF_DYNALLOCED.
	 * Этот вызов внутри функции Unload играет важную роль,
	 * сравнимую с Command_Unregister и EVENT_UNREGISTER.
	*/
	Block_UpdateDefinition(&myBlock);
	Block_UpdateDefinition(&myExtendedBlock);
	Block_UpdateDefinition(myDynBlock);

	/*
	 * Возврат true говорит о том, что
	 * плагин может быть выгружен в данный
	 * момент без ущерба работоспособности
	 * сервера. Если функция вернёт false
	 * то плагин останется в памяти
	 * нетронутым.
	*/
	return true;
}
