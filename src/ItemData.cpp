#include "ItemData.h"

#include "skse64/GameAPI.h"  // g_thePlayer
#include "skse64/GameExtraData.h"  // InventoryEntryData
#include "skse64/GameFormComponents.h"  // BGSBipedObjectForm, TESEnchantableForm
#include "skse64/GameForms.h"  // TESForm
#include "skse64/GameObjects.h"  // TESObjectARMO, TESObjectBOOK, TESObjectMISC, TESObjectWEAP, TESAmmo, AlchemyItem, TESSoulGem
#include "skse64/GameRTTI.h"  // DYNAMIC_CAST

#include "ExtendDataListVisitor.h"  // ExtendDataListVisitor
#include "Keywords.h"  // keywords

#include "RE/BGSBipedObjectForm.h"  // RE::BGSBipedObjectForm


namespace QuickLootRE
{
	ItemData::ItemData(InventoryEntryData* a_entryData) :
		_entryData(a_entryData),
		_name(""),
		_count(0),
		_value(0),
		_weight(0.0),
		_type(kType_None),
		_isStolen(false),
		_isEnchanted(false),
		_priority(kPriority_Key)
	{
		_name = CALL_MEMBER_FN(_entryData, GenerateName)();
		_count = _entryData->countDelta;
		_value = CALL_MEMBER_FN(_entryData, GetValue)();
		_weight = getWeight();
		_type = getType();
		_isStolen = CALL_MEMBER_FN(_entryData, IsOwnedBy)((*g_thePlayer), false);
		_isEnchanted = getEnchanted();
		_priority = getPriority();
	}


	ItemData::ItemData(InventoryEntryData* a_entryData, SInt32 a_count) :
		_entryData(a_entryData),
		_name(""),
		_count(a_count),
		_value(0),
		_weight(0.0),
		_type(kType_None),
		_isStolen(false),
		_isEnchanted(false),
		_priority(kPriority_Key)
	{
		_name = CALL_MEMBER_FN(_entryData, GenerateName)();
		_value = CALL_MEMBER_FN(_entryData, GetValue)();
		_weight = getWeight();
		_type = getType();
		_isStolen = CALL_MEMBER_FN(_entryData, IsOwnedBy)((*g_thePlayer), false);
		_isEnchanted = getEnchanted();
		_priority = getPriority();
	}


	ItemData::~ItemData()
	{}


	InventoryEntryData* ItemData::entryData() const
	{
		return _entryData;
	}


	const char* ItemData::name() const
	{
		return _name;
	}


	SInt32 ItemData::count() const
	{
		return _count;
	}


	SInt32 ItemData::value() const
	{
		return _value;
	}


	float ItemData::weight() const
	{
		return _weight;
	}


	const char* ItemData::icon() const
	{
		return _strIcons[_type];
	}


	bool ItemData::isStolen() const
	{
		return _isStolen;
	}


	TESForm* ItemData::form() const
	{
		return _entryData->type;
	}


	float ItemData::getWeight()
	{
		TESWeightForm* weightForm = DYNAMIC_CAST(_entryData->type, TESForm, TESWeightForm);
		return weightForm ? weightForm->weight : 0.0;
	}


	ItemData::Type ItemData::getType()
	{
		switch (_entryData->type->formType) {
		case kFormType_ScrollItem:
			return kType_DefaultScroll;
		case kFormType_Armor:
			return getTypeArmor(static_cast<TESObjectARMO*>(_entryData->type));
		case kFormType_Book:
			return getTypeBook(static_cast<TESObjectBOOK*>(_entryData->type));
		case kFormType_Ingredient:
			return kType_DefaultIngredient;
		case kFormType_Light:
			return kType_MiscTorch;
		case kFormType_Misc:
			return getTypeMisc(static_cast<TESObjectMISC*>(_entryData->type));
		case kFormType_Weapon:
			return getTypeWeapon(static_cast<TESObjectWEAP*>(_entryData->type));
		case kFormType_Ammo:
			return (static_cast<TESAmmo*>(_entryData->type)->isBolt()) ? kType_WeaponBolt : kType_WeaponArrow;
		case kFormType_Key:
			return kType_DefaultKey;
		case kFormType_Potion:
			return getTypePotion(static_cast<AlchemyItem*>(_entryData->type));
		case kFormType_SoulGem:
			return getTypeSoulGem(static_cast<TESSoulGem*>(_entryData->type));
		default:
			return kType_None;
		}
	}


	ItemData::Type ItemData::getTypeArmor(TESObjectARMO* a_armor)
	{
		static Type types[] = {
			kType_LightArmorBody,		// 0
			kType_LightArmorHead,
			kType_LightArmorHands,
			kType_LightArmorForearms,
			kType_LightArmorFeet,
			kType_LightArmorCalves,
			kType_LightArmorShield,
			kType_LightArmorMask,

			kType_ArmorBody,			// 8
			kType_ArmorHead,
			kType_ArmorHands,
			kType_ArmorForearms,
			kType_ArmorFeet,
			kType_ArmorCalves,
			kType_ArmorShield,
			kType_ArmorMask,

			kType_ClothingBody,			// 16
			kType_ClothingHead,
			kType_ClothingHands,
			kType_ClothingForearms,
			kType_ClothingFeet,
			kType_ClothingCalves,
			kType_ClothingShield,
			kType_ClothingMask,

			kType_ArmorAmulet,			// 24
			kType_ArmorRing,
			kType_Circlet,

			kType_DefaultArmor			// 27
		};

		UInt32 index = 0;
		RE::BGSBipedObjectForm* bipedObj = reinterpret_cast<RE::BGSBipedObjectForm*>(&a_armor->bipedObject);

		if (bipedObj->IsLightArmor()) {
			index = 0;
		} else if (bipedObj->IsHeavyArmor()) {
			index = 8;
		} else {
			if (a_armor->keyword.HasKeyword(VendorItemClothing)) {
				index = 16;
			} else if (a_armor->keyword.HasKeyword(VendorItemJewelry)) {
				if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Amulet)) {
					index = 24;
				} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Ring)) {
					index = 25;
				} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Circlet)) {
					index = 26;
				} else {
					index = 27;
				}
			} else {
				index = 27;
			}
		}

		if (index >= 24) {
			return types[index];
		}

		if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Body | RE::BGSBipedObjectForm::kPart_Tail)) {
			index += 0;			// body
		} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Head | RE::BGSBipedObjectForm::kPart_Hair | RE::BGSBipedObjectForm::kPart_LongHair)) {
			index += 1;			// head
			if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Circlet)) {
				index += 6;		// mask
			}
		} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Hands)) {
			index += 2;			// hands
		} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Forearms)) {
			index += 3;			// forearms
		} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Feet)) {
			index += 4;			// feet
		} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Calves)) {
			index += 5;			// calves
		} else if (bipedObj->HasPartOf(RE::BGSBipedObjectForm::kPart_Shield)) {
			index += 6;			// shield
		} else {
			index = 27;
		}

		return types[index];
	}


	ItemData::Type ItemData::getTypeBook(TESObjectBOOK* a_book)
	{
		if (a_book->data.type == 0xFF || a_book->keyword.HasKeyword(VendorItemRecipe)) {
			return kType_BookNote;
		} else if (a_book->keyword.HasKeyword(VendorItemSpellTome)) {
			return kType_BookTome;
		} else {
			return kType_DefaultBook;
		}
	}


	ItemData::Type ItemData::getTypeMisc(TESObjectMISC* a_misc)
	{
		if (a_misc->keyword.HasKeyword(VendorItemAnimalHide)) {
			return kType_MiscHide;
		} else if (a_misc->keyword.HasKeyword(VendorItemDaedricArtifact)) {
			return kType_MiscArtifact;
		} else if (a_misc->keyword.HasKeyword(VendorItemGem)) {
			return kType_MiscGem;
		} else if (a_misc->keyword.HasKeyword(VendorItemAnimalPart)) {
			return kType_MiscRemains;
		} else if (a_misc->keyword.HasKeyword(VendorItemOreIngot)) {
			return kType_MiscIngot;
		} else if (a_misc->keyword.HasKeyword(VendorItemClutter)) {
			return kType_MiscClutter;
		} else if (a_misc->keyword.HasKeyword(VendorItemFireword)) {
			return kType_MiscWood;
		} else {
			switch (a_misc->formID) {
			case kFormID_LockPick:
				return kType_MiscLockPick;
			case kFormID_Gold:
				return kType_MiscGold;
			case kFormID_Leather01:
				return kType_MiscLeather;
			case kFormID_LeatherStrips:
				return kType_MiscStrips;
			case kFormID_DragonClawIron:
			case kFormID_DragonClawGoldenE3:
			case kFormID_DragonClawGoldenMS13:
			case kFormID_DragonClawCoral:
			case kFormID_DragonClawIvory:
			case kFormID_DragonClawRuby:
			case kFormID_DragonClawSapphire:
			case kFormID_DragonClawEmerald:
			case kFormID_DragonClawGlass:
			case kFormID_DragonClawEbony:
			case kFormID_DragonClawDiamond:
				return kType_MiscDragonClaw;
			default:
				return kType_DefaultMisc;
			}
		}
	}


	ItemData::Type ItemData::getTypeWeapon(TESObjectWEAP* a_weap)
	{
		switch (a_weap->type()) {
		case TESObjectWEAP::GameData::kType_OneHandSword:
			return kType_WeaponSword;
		case TESObjectWEAP::GameData::kType_OneHandDagger:
			return kType_WeaponDagger;
		case TESObjectWEAP::GameData::kType_OneHandAxe:
			return kType_WeaponWarAxe;
		case TESObjectWEAP::GameData::kType_OneHandMace:
			return kType_WeaponMace;
		case TESObjectWEAP::GameData::kType_TwoHandSword:
			return kType_WeaponGreatSword;
		case TESObjectWEAP::GameData::kType_TwoHandAxe:
			if (a_weap->keyword.HasKeyword(WeapTypeWarhammer)) {
				return kType_WeaponHammer;
			} else {
				return kType_WeaponBattleAxe;
			}
		case TESObjectWEAP::GameData::kType_Bow:
			return kType_WeaponBow;
		case TESObjectWEAP::GameData::kType_Staff:
			return kType_WeaponStaff;
		case TESObjectWEAP::GameData::kType_CrossBow:
			return kType_WeaponCrossbow;
		default:
			return kType_DefaultWeapon;
		}
	}


	ItemData::Type ItemData::getTypePotion(AlchemyItem* a_potion)
	{
		ItemData::Type type = ItemData::kType_DefaultPotion;

		if (a_potion->IsFood()) {
			return (a_potion->itemData.useSound && a_potion->itemData.useSound->formID == kFormID_ITMPotionUse) ? kType_FoodWine : kType_DefaultFood;
		} else if (a_potion->IsPoison()) {
			return kType_PotionPoison;
		} else {
			MagicItem::EffectItem* effectItem = CALL_MEMBER_FN(a_potion, GetCostliestEffectItem)(5, false);
			if (effectItem && effectItem->mgef) {
				switch (effectItem->mgef->properties.primaryValue) {
				case kActorValue_Health:
					return kType_PotionHealth;
				case kActorValue_Magicka:
					return kType_PotionMagic;
				case kActorValue_Stamina:
					return kType_PotionStam;
				case kActorValue_FireResist:
					return kType_PotionFire;
				case kActorValue_ElectricResist:
					return kType_PotionShock;
				case kActorValue_FrostResist:
					return kType_PotionFrost;
				}
			}
		}
		return kType_DefaultPotion;
	}


	ItemData::Type ItemData::getTypeSoulGem(TESSoulGem* a_gem)
	{
		if (a_gem->formID == kFormID_DA01SoulGemBlackStar || a_gem->formID == kFormID_DA01SoulGemAzurasStar) {
			return kType_SoulGemAzura;
		} else {
			UInt32 xSoulSize = CALL_MEMBER_FN(_entryData, GetSoulLevel)();
			if (a_gem->gemSize < 4) {
				if (a_gem->soulSize >= a_gem->gemSize || xSoulSize >= a_gem->gemSize) {
					return kType_SoulGemFull;
				} else if (a_gem->soulSize > 0 || xSoulSize > 0) {
					return kType_SoulGemPartial;
				} else {
					return kType_SoulGemEmpty;
				}
			} else {
				if (a_gem->soulSize >= a_gem->gemSize || xSoulSize >= a_gem->gemSize) {
					return kType_SoulGemGrandFull;
				} else if (a_gem->soulSize > 0 || xSoulSize > 0) {
					return kType_SoulGemGrandPartial;
				} else {
					return kType_SoulGemGrandEmpty;
				}
			}
		}
	}


	bool ItemData::getEnchanted()
	{
		if (_entryData->extendDataList) {
			ExtendDataListVisitor visitor({ kExtraData_Enchantment }, { });
			_entryData->extendDataList->Visit(visitor);
			if (visitor.found()) {
				return true;
			}
		}
		TESEnchantableForm* enchantForm = DYNAMIC_CAST(_entryData->type, TESForm, TESEnchantableForm);
		if (enchantForm && enchantForm->enchantment) {
			return true;
		}
		return false;
	}


	ItemData::Priority ItemData::getPriority()
	{
		switch (_entryData->type->formType) {
		case kFormType_Ammo:
			return kPriority_Ammo;
		case kFormType_SoulGem:
			return kPriority_SoulGem;
		case kFormType_Potion:
			switch (_type) {
			case kType_DefaultFood:
			case kType_FoodWine:
			case kType_FoodBeer:
				return kPriority_Food;
			case kType_PotionPoison:
				return kPriority_Poison;
			default:
				return kPriority_Potion;
			}
		case kFormType_Weapon:
			return (_isEnchanted) ? kPriority_EnchantedWeapon : kPriority_Weapon;
		case kFormType_Armor:
			if (_isEnchanted) {
				return kPriority_EnchantedArmor;
			} else if (_type == kType_ArmorAmulet) {
				return kPriority_Amulet;
			} else if (_type == kType_ArmorRing) {
				return kPriority_Ring;
			} else {
				return kPriority_Armor;
			}
		case kFormType_Key:
			return kPriority_Key;
		case kFormType_Misc:
			switch (_type) {
			case kType_MiscGold:
				return kPriority_Gold;
			case kType_MiscLockPick:
				return kPriority_LockPick;
			case kType_MiscGem:
				return kPriority_Gem;
			default:
				return kPriority_Other;
			}
		default:
			return kPriority_Other;
		}
	}


	int CompareByStolen(const ItemData& a_lhs, const ItemData& a_rhs)
	{
		SInt32 valueLHS = a_lhs._isStolen ? 1 : 0;
		SInt32 valueRHS = a_rhs._isStolen ? 1 : 0;

		return valueLHS - valueRHS;
	}


	static int CompareByType(const ItemData& a_lhs, const ItemData& a_rhs)
	{
		return a_lhs._priority - a_rhs._priority;
	}


	static int CompareByName(const ItemData& a_lhs, const ItemData& a_rhs)
	{
		return strcmp(a_lhs._name, a_rhs._name);
	}


	static int CompareByValue(const ItemData& a_lhs, const ItemData& a_rhs)
	{
		SInt32 valueLHS = a_lhs._value;
		SInt32 valueRHS = a_rhs._value;
		return valueLHS - valueRHS;
	}


	static int CompareByCount(const ItemData& a_lhs, const ItemData& a_rhs)
	{
		SInt32 valueLHS = a_lhs._count;
		SInt32 valueRHS = a_rhs._count;
		return valueLHS - valueRHS;
	}


	const char* ItemData::_strIcons[] = {
		"none",					// 00
		"default_weapon",
		"weapon_sword",
		"weapon_greatsword",
		"weapon_daedra",
		"weapon_dagger",
		"weapon_waraxe",
		"weapon_battleaxe",
		"weapon_mace",
		"weapon_hammer",
		"weapon_staff",			// 10
		"weapon_bow",
		"weapon_arrow",
		"weapon_pickaxe",
		"weapon_woodaxe",
		"weapon_crossbow",
		"weapon_bolt",
		"default_armor",
		"lightarmor_body",
		"lightarmor_head",
		"lightarmor_hands",		// 20
		"lightarmor_forearms",
		"lightarmor_feet",
		"lightarmor_calves",
		"lightarmor_shield",
		"lightarmor_mask",
		"armor_body",
		"armor_head",
		"armor_hands",
		"armor_forearms",
		"armor_feet",			// 30
		"armor_calves",
		"armor_shield",
		"armor_mask",
		"armor_bracer",
		"armor_daedra",
		"clothing_body",
		"clothing_robe",
		"clothing_head",
		"clothing_pants",
		"clothing_hands",		// 40
		"clothing_forearms",
		"clothing_feet",
		"clothing_calves",
		"clothing_shoes",
		"clothing_shield",
		"clothing_mask",
		"armor_amulet",
		"armor_ring",
		"armor_circlet",
		"default_scroll",		// 50
		"default_book",
		"default_book_read",
		"book_tome",
		"book_tome_read",
		"book_journal",
		"book_note",
		"book_map",
		"default_food",
		"food_wine",
		"food_beer",			// 60
		"default_ingredient",
		"default_key",
		"key_house",
		"default_potion",
		"potion_health",
		"potion_stam",
		"potion_magic",
		"potion_poison",
		"potion_frost",
		"potion_fire",			// 70
		"potion_shock",
		"default_misc",
		"misc_artifact",
		"misc_clutter",
		"misc_lockpick",
		"misc_soulgem",
		"soulgem_empty",
		"soulgem_partial",
		"soulgem_full",
		"soulgem_grandempty",	// 80
		"soulgem_grandpartial",
		"soulgem_grandfull",
		"soulgem_azura",
		"misc_gem",
		"misc_ore",
		"misc_ingot",
		"misc_hide",
		"misc_strips",
		"misc_leather",
		"misc_wood",			// 90
		"misc_remains",
		"misc_trollskull",
		"misc_torch",
		"misc_goldsack",
		"misc_gold",
		"misc_dragonclaw"
	};
}
