#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace RE;

PlayerCharacter* p = nullptr;
BSScript::IVirtualMachine* vm = nullptr;
TESDataHandler* dataHandler = nullptr;
BGSProjectile* cProj = nullptr;
ActorValueInfo* aPenet = nullptr;

namespace temp
{
	struct RefrOrInventoryObj
	{
		TESObjectREFR* refr;
		TESObjectREFR* inv;
		uint16_t id;
	};

}

const char* GetModel(TESForm* send)
{
	using func_t = decltype(&GetModel);
	REL::Relocation<func_t> func{ REL::ID(1039522) };
	return func(send);
}

float GetRandomfloat(float a, float b)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(a, b);
	float randNum = dist(mt);

	return randNum;
}

void HitHead(std::monostate, Actor* a)
{
	//logger::info("실행");
	if (!a)
		return;

	BSTSmartPointer<BipedAnim> tempanims = a->biped;
	if (!tempanims)
		return;

	if (!a->IsDead(true)) {
		//logger::info("안죽었어");
		if (!a->GetHostileToActor(p)) {
			//logger::info("아직 적대적인 적이 아님");
			return;
		}

		BSTArray<EquippedItem>* equipped = &p->currentProcess->middleHigh->equippedItems;
		if (!equipped)
			return;

		if (equipped->size() != 0 && (*equipped)[0].item.instanceData) {
			TESObjectWEAP* weap = (TESObjectWEAP*)(*equipped)[0].item.object;

			if (!weap)
				return;

			float vSpeed = weap->weaponData.rangedData->boltChargeSeconds;
			bool isBolt = vSpeed > 0.5;
			
			if (!isBolt) {
				float fPenet = p->GetActorValue(*aPenet);
				if (GetRandomfloat(0, 1) > 0.4 + (fPenet / 100)) {
					//logger::info("볼트액션이 아니거나 확률 계산에 실패");
					return;
				}
			}
		}
	}

	for (int i = 0; i <= 16; i += 16) {
		TESForm* form = tempanims->object[i].parent.object;
		if (form) {
			if (tempanims->object[i].skinned) {
				if (form->GetPlayable(tempanims->object[i].parent.instanceData.get())) {
					//logger::info("{}번 슬롯은 플레이어블", i);
					a->UnequipArmorFromSlot((BIPED_OBJECT)i, true);

					const char* ModelPath = GetModel(form);
					if (!ModelPath) {
						//logger::info("모델 경로 못구함");
					} else {
						TESModel* cModel = (TESModel*)cProj;
						cModel->model = ModelPath;

						cProj->data.speed = GetRandomfloat(430, 550);

						NiAVObject* head = a->currentProcess->middleHigh->headNode;
						NiPoint3 pPoint = p->GetPosition();
						NiPoint3 aPoint = a->GetPosition();

						NiPoint3 headPoint = head->world.translate;

						double px = pPoint.x;
						double py = pPoint.y;
						double ax = aPoint.x;
						double ay = aPoint.y;

						double xDiff = ax - px;
						double yDiff = ay - py;
						double angleInRadians = -std::atan2(yDiff, xDiff) + (M_PI / 2.0) + GetRandomfloat(-0.3, 0.3);

						double heightDiff = pPoint.z - aPoint.z;
						double angleInZAxis = atan(heightDiff / pPoint.GetDistance(aPoint)) - GetRandomfloat(0.40, 0.75);
						if (angleInZAxis < -1.4) {
							angleInZAxis = -1.4;
						}

						NiPoint3 shotAngle(angleInZAxis, 0, angleInRadians);
						ObjectRefHandle tempproj = dataHandler->CreateProjectileAtLocation(cProj, headPoint, shotAngle, a->parentCell, a->parentCell->worldSpace);
					}
					return;
				}
			}
		}
	}
}

void OnF4SEMessage(F4SE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case F4SE::MessagingInterface::kGameLoaded:
		dataHandler = RE::TESDataHandler::GetSingleton();
		p = PlayerCharacter::GetSingleton();
		cProj = (BGSProjectile*)dataHandler->LookupForm(0x800, "HH_HeadHunting.esp");
		aPenet = (ActorValueInfo*)dataHandler->LookupForm(0x00097341, "fallout4.esm");
		break;
	}
}

bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* a_vm)
{
	vm = a_vm;

	REL::IDDatabase::Offset2ID o2i;
	logger::info("0x0x140b0e0: {}", o2i(0x140b0e0));

	a_vm->BindNativeMethod("HH_HeadHunting"sv, "HitHead"sv, HitHead);

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format("{}.log", Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("Global Log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::trace);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%^%l%$] %v"s);

	logger::info("{} v{}", Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical("unsupported runtime v{}", ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	if (papyrus)
		papyrus->Register(RegisterPapyrusFunctions);

	const F4SE::MessagingInterface* message = F4SE::GetMessagingInterface();
	if (message)
		message->RegisterListener(OnF4SEMessage);

	return true;
}
