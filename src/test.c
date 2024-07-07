/*
 * Here is an example of minimal
 * CServer plugin.
*/

#include <core.h>
#include <str.h>
#include <log.h>
#include <list.h>
#include <client.h>
#include <block.h>
#include <event.h>
#include <command.h>
#include <world.h>
#include <plugin.h>

// Plugin headers
#include "test.h"

/*
 * See file src/types/block.h
 * for more information.
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

static cs_bool enabled = false;

/*
 * This event fires every time the user
 * sends a message to the chat. Message
 * and its type are writable.
*/
static void onmesgfunc(onMessage *obj) {
  if(enabled)
	obj->type = MESSAGE_TYPE_ANNOUNCE;
}

/*
 * This event fires when some world got
 * loaded from disk.
*/
void onworldload(World *world) {
	/*
	 * The BlockDefinitions extension
	 * allows you to register custom blocks
	 * for clients. The Block_Define function
	 * accepts BlockDef struct pointer.
	 * The BlockDef struct contains 4 fields:
	 * flags, block id, its name and parameters.
	 * If the block being created uses an extended
	 * set of parameters (see BlockDefinitionsExt 
	 * on wiki.vg) then the BDF_EXTENDED flag must
	 * be set.
	*/
	Block_Define(world, BLOCK_ID, &myBlock);
	Block_Define(world, BLOCK_ID_EXT, &myExtendedBlock);
}

/*
 * If the command was called from the console, the ccdata->caller field will be NULL.
 * The ccdata->args field will also be NULL if no arguments were passed.
*/
COMMAND_FUNC(PlugTest) {
	COMMAND_PRINT("This command registred by testplugin." DLIB_EXT);
}

COMMAND_FUNC(Atoggle) {
	enabled ^= 1;
	COMMAND_PRINTF("Announce chat %s", enabled ? "&aenabled" : "&cdisabled");
}

COMMAND_FUNC(Announce) {
	if(!ccdata->args) return false;
	Client_Chat(CLIENT_BROADCAST, MESSAGE_TYPE_ANNOUNCE, ccdata->args);
	COMMAND_PRINT("Announcement sent.");
}

COMMAND_FUNC(SelfDestroy) {
	Command_Unregister(ccdata->command);
	COMMAND_PRINT("This command can no longer be called");
}

COMMAND_FUNC(ClientOnly) {
	COMMAND_PRINTF("Client-only command called by %s", Client_GetName(ccdata->caller));
}

/*
 * This macro sets the plugin and server's API version.
 * The server will not be able to load the plugin
 * without calling this macro.
*/
Plugin_SetVersion(1);

Event_DeclareBunch (events) { // This macro creates an array to be passed to the Event_*Bunch functions
	EVENT_BUNCH_ADD('v', EVT_ONMESSAGE, onmesgfunc)
	EVENT_BUNCH_ADD('v', EVT_ONWORLDADDED, onworldload)

	EVENT_BUNCH_END
};

Command_DeclareBunch (commands) {
	/*
	 * Each chat command has flags.
	 * Here is a list of currently suppored flags:
	 *  - CMDF_NONE - this command has no calling rules
	 *  - CMDF_OP - this command can only be called by the server operator
	 *  - CMDF_CLIENT - this command can only be called by the client
	 *  this means that calling it from the console is impossible
	 * CMDF_OP and CMDF_CLIENT can be combined (CMDF_OP | CMD_CLIENT)
	 * means that the command can only be called by the operator player.
	*/
	COMMAND_BUNCH_ADD(PlugTest, CMDF_NONE, "Test command")
	COMMAND_BUNCH_ADD(Atoggle, CMDF_OP, "Test command")
	COMMAND_BUNCH_ADD(Announce, CMDF_OP, "Test command")
	COMMAND_BUNCH_ADD(SelfDestroy, CMDF_NONE, "Test command")
	COMMAND_BUNCH_ADD(ClientOnly, CMDF_CLIENT, "Test command")

	COMMAND_BUNCH_END
};

cs_bool Plugin_Load(void) { // Main plugin function, will be called once when the plugin is loaded
	Event_RegisterBunch(events);
	Command_RegisterBunch(commands);

	// Every Log function acts like printf
	Log_Info("Test plugin loaded");
	Log_Debug("It's a debug message");
	Log_Warn("It's a warning message");

	/*
	 * If the plugin was loaded *after* the start
	 * of the main server loop, then you need to
	 * iterate through all the loaded worlds.
	*/
	AListField *tmp;
	List_Iter(tmp, World_Head)
		onworldload(AList_GetValue(tmp).ptr);

	/*
	 * To update an already registred block, you
	 * must set the BDF_UPDATED flag and then call
	 * the Block_UpdateDifinition function.
	*/
	Block_UpdateDefinition(&myBlock);
	Block_UpdateDefinition(&myExtendedBlock);

	/*
	 * If the function returned false, the server will
	 * call the Plugin_Unload function of the plugin
	 * and unload its library.
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
	Event_UnregisterBunch(events);
	Command_UnregisterBunch(commands);

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

	/*
	 * If this function returns false,
	 * the plugin will not be unloaded.
	 * The return value of this function
	 * will be ignored if the @force
	 * argument is set to true.
	*/
	return true;
}
