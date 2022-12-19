#include "common/IDebugLog.h"  // IDebugLog
#include "skse64_common/SafeWrite.h"
#include "skse64_common/skse_version.h"  // RUNTIME_VERSION
#include "skse64/PluginAPI.h"  // SKSEInterface, PluginInfo

#include <ShlObj.h>  // CSIDL_MYDOCUMENTS

#include "version.h"


// SKSE globals
static PluginHandle	g_pluginHandle = kPluginHandle_Invalid;
static SKSEMessagingInterface *g_messaging = nullptr;


extern "C" {
	// Listener for SKSE Messages
	void OnSKSEMessage(SKSEMessagingInterface::Message* msg)
	{
		if (msg) {
			if (msg->type == SKSEMessagingInterface::kMessage_PostLoad) {
				HMODULE handle = GetModuleHandle("DynamicAnimationReplacer");
				if (handle == NULL) {
					_MESSAGE("Could not find DynamicAnimationReplacer.dll. Doing nothing in this case.");
					return;
				}

				_MESSAGE("Patching DAR...");

				uintptr_t address = (uintptr_t)handle + 0x5593;
				char *bytes = "\xFF\xA0\x28\x07\x00\x00"; // jmp qword ptr ds:[rax+0x728]
				SafeWriteBuf(address, bytes, 6);

				_MESSAGE("DAR successfully patched");
			}
		}
	}

	bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info)
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim VR\\SKSE\\dar_vr_fix.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_Message);
		gLog.SetLogLevel(IDebugLog::kLevel_Message);

		_MESSAGE("DAR VR Fix v%s", DAR_VR_FIX_VERSION_VERSTRING);

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "DAR VR Fix";
		info->version = DAR_VR_FIX_VERSION_MAJOR;

		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor) {
			_FATALERROR("[FATAL ERROR] Loaded in editor, marking as incompatible!\n");
			return false;
		}
		else if (skse->runtimeVersion != RUNTIME_VR_VERSION_1_4_15) {
			_FATALERROR("[FATAL ERROR] Unsupported runtime version %08X!\n", skse->runtimeVersion);
			return false;
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{	// Called by SKSE to load this plugin
		_MESSAGE("DAR VR Fix loaded");

		_MESSAGE("Registering for SKSE messages");
		g_messaging = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging);
		g_messaging->RegisterListener(g_pluginHandle, "SKSE", OnSKSEMessage);

		return true;
	}
};
