#include "Procedure.h"
#include "ToothContainer.h"
#include "ToothUtils.h"

using namespace Dental;

const ToothIndex& Procedure::getToothIndex() const
{
	static ToothIndex dummy;

	if (affectedTeeth.index() != 1) {
		return dummy;
	}

	return std::get<ToothIndex>(affectedTeeth);
}

Procedure::Scope Procedure::getScope(Type type)
{
	static const std::map<Type, Scope> map = {

		{ Type::General, Scope::AllOrNone},
		{ Type::Depuratio, Scope::AllOrNone},
		{ Type::DenturePair, Scope::AllOrNone},
		{ Type::Restoration, Scope::SingleTooth},
		{ Type::RemoveRestoration, Scope::SingleTooth},
		{ Type::ToothNonSpecific, Scope::SingleTooth},
		{ Type::DepuratioTooth, Scope::SingleTooth},
		{ Type::Extraction, Scope::SingleTooth},
		{ Type::Implant, Scope::SingleTooth},
		{ Type::Endodontic, Scope::SingleTooth},
		{ Type::Post, Scope::SingleTooth},
		{ Type::RemovePost, Scope::SingleTooth},
		{ Type::PostCore, Scope::SingleTooth},
		{ Type::PostCrown, Scope::SingleTooth},
		{ Type::Bridge, Scope::Range},
		{ Type::RemoveCrownOrBridge, Scope::Range},
		{ Type::Denture, Scope::Range},
		{ Type::Splint, Scope::Range},
		{ Type::MultipleExtraction, Scope::Range},
		{ Type::Crown, Scope::SingleTooth}
	};

	return map.at(type);
}

bool Procedure::affectsToothIdx(int toothIdx) const
{
	auto scope = getScope();

	switch (scope)
	{
	case Scope::SingleTooth:
	{
		auto& toothIdxObj = getToothIndex();
		return toothIdxObj.index == toothIdx;
	}
	case Scope::Range:
	{
		auto& range = std::get<ConstructionRange>(affectedTeeth);
		return toothIdx >= range.toothFrom && toothIdx <= range.toothTo;
	}
	}

	return false;
}

std::vector<int> Procedure::getArrayIndexes() const
{
	std::vector<int> result;

	auto scope = getScope(type);

	if (scope == Scope::AllOrNone) return result;

	if (scope == Scope::SingleTooth) return { getToothIndex().index };

	auto& range = std::get<ConstructionRange>(affectedTeeth);
	result.reserve(range.getTeethCount());

	for (int i = range.toothFrom; i < range.toothTo + 1; i++) {
		result.push_back(i);
	}

	return result;
}

std::vector<const Tooth*> Procedure::applyProcedure(ToothContainer& teeth) const
{
	std::vector<int> teeth_indexes;
	std::vector<int> dsn_indexes;

	auto scope = getScope(type);

	//first we determine which teeth are affected by the procedure

	switch (scope)
	{
		case Scope::SingleTooth:
			{
				auto& toothIdx = getToothIndex();

				toothIdx.supernumeral ?
					dsn_indexes.push_back(toothIdx.index)
					:
					teeth_indexes.push_back(toothIdx.index);
			}
			break;

		case Scope::Range:
		{
			auto& range = std::get<ConstructionRange>(affectedTeeth);

			for (int i = range.toothFrom; i < range.toothTo + 1; i++) {

		
				if (type == Type::Denture) {

					if (teeth[i].canHaveADenture()) {
						teeth_indexes.push_back(i);
					}
					continue;
				}


				if (type == Type::MultipleExtraction) {

					if (!teeth[i][Missing]) {
						teeth_indexes.push_back(i);
					}

					if (teeth[i][HasSupernumeral] && !teeth[i].getSupernumeral()[Missing]) {
						dsn_indexes.push_back(i);
					}

					continue;
				}

				teeth_indexes.push_back(i);


				if (type == Type::RemoveCrownOrBridge) {

					if (teeth[i].getSupernumeral().hasStatus(Crown)) {
						dsn_indexes.push_back(i);
					}
				}
			}
			
		}
		break;

		case Scope::AllOrNone:
		{
			
			if (type == Type::Depuratio) {

				for (int i = 0; i < Dental::teethCount; i++) {

					if (teeth[i][Calculus]) { teeth_indexes.push_back(i); }
					if (teeth[i].getSupernumeral()[Calculus]) { dsn_indexes.push_back(i); }
				}
			}

			if (type == Type::DenturePair) {

				std::array range = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,
								17,18,19,20,21,22,23,24,25,26,27,28,29,30 };

				for (auto i : range) {
					if (teeth[i].canHaveADenture()) {
						teeth_indexes.push_back(i);
					}
				}


			}
		}

	}

	//then define how the status should be changed

	struct StatusChange {
		StatusType type;
		int code;
		bool state;
		int lpk = -1; //made by this dentist
	};

	std::vector<StatusChange> changes;

	switch (type)
	{
		case Type::General:
			break;
			case Type::Depuratio:
			changes.push_back({ StatusType::General, Calculus, false });
			break;
		case Type::DenturePair:
			changes.push_back({ StatusType::General, Denture, true, Denture });
			break;
		case Type::Restoration:
		{
			changes.push_back({ StatusType::General, Fracture, false });

			auto& data = std::get<RestorationData>(param);

			for (int i = 0; i < data.surfaces.size(); i++) {

				if (!data.surfaces[i]) continue;

				changes.push_back({ StatusType::Caries, i, false });
				changes.push_back({ StatusType::NonCariesLesion, i, false });
				changes.push_back({ StatusType::Restoration, i, true, Restoration });
			}

			if (data.post) {
				changes.push_back({ StatusType::General, Post, true, Post });
			}
		}
			break;
		case Type::RemoveRestoration:
		{
			auto& data = std::get<RestorationData>(param);

			for (int i = 0; i < data.surfaces.size(); i++) {

				if (!data.surfaces[i]) continue;

				changes.push_back({ StatusType::Restoration, i, false });
				changes.push_back({ StatusType::DefectiveRestoration, i, false });
				changes.push_back({ StatusType::NonCariesLesion, i, true });
			}

			if (data.post) {
				changes.push_back({ StatusType::General, Post, false });
			}
		}
			break;
		case Type::ToothNonSpecific:
			break;
		case Type::DepuratioTooth:
			changes.push_back({ StatusType::General, Calculus, false });
			break;
		case Type::Extraction:
		{
			bool hasDenture = teeth.at(getToothIndex()).hasStatus(Denture);

			changes.push_back({ StatusType::General, Missing, true, Missing });

			if (hasDenture) //since extraction remove denture status
			{
				changes.push_back({ StatusType::General, Denture, true });
			}

			break;
		}
		case Type::Implant:
			changes.push_back({ StatusType::General, Implant, true, Implant });
			break;
		case Type::Endodontic:
			changes.push_back({ StatusType::General, RootCanal, true, RootCanal });
			break;
		case Type::Post:
			changes.push_back({ StatusType::General, Post, true, Post });
			break;
		case Type::RemovePost:
			changes.push_back({ StatusType::General, Post, false });
			break;
		case Type::PostCore:
			changes.push_back({ StatusType::General, Root, true });
			changes.push_back({ StatusType::General, Fracture, false });
			changes.push_back({ StatusType::General, Post, true, Post });
			for (int i = 0; i < 6; i++) {
				changes.push_back({ StatusType::Restoration, i, true, Restoration });
			}
			break;
		case Type::PostCrown:
			changes.push_back({ StatusType::General, Fracture, false });
			changes.push_back({ StatusType::General, Post, true, Post });
			changes.push_back({ StatusType::General, Crown, true, Crown });
			break;
		case Type::Crown:
		{
			changes.push_back({ StatusType::General, Fracture, false });
			auto status = Crown;
			changes.push_back({ StatusType::General, status, true, status });
		}
			break;
		case Type::Bridge:
			changes.push_back({ StatusType::General, Fracture, false });
			changes.push_back({ StatusType::General, Bridge, true, Bridge });
			break;
		case Type::RemoveCrownOrBridge:
			changes.push_back({ StatusType::General, Crown, false});
			changes.push_back({ StatusType::General, Bridge, false });
			changes.push_back({ StatusType::General, Splint, false });
			break;
		case Type::Denture:
			changes.push_back({ StatusType::General, Denture, true, Denture });
			break;
		case Type::Splint:
			changes.push_back({ StatusType::General, Splint, true, Splint });
			break;
		case Type::MultipleExtraction:
			changes.push_back({ StatusType::General, Missing, true, Missing });
			break;
		case Type::MaxCount:
			break;
		default:
			break;
	}

	//the meat grinder:

	for (auto& change : changes) {

		teeth.setStatus(teeth_indexes, change.type, change.code, change.state, false);
		teeth.setStatus(dsn_indexes, change.type, change.code, change.state, true);

		if (change.lpk != -1) {
			for (int i : teeth_indexes) teeth.at(i).setDentistRowid(change.code, dentist_rowid);
			for (int i : dsn_indexes) teeth.at(i).getSupernumeral().setDentistRowid(change.code, dentist_rowid);
		}
	}

	//generating the result with the affected teeth

	std::vector<const Tooth*> result;
	
	for (auto i : teeth_indexes) {
		result.push_back(&teeth.at(i));
	}

	for (auto i : dsn_indexes) {
		result.push_back(&teeth.at(i).getSupernumeral());
	}

	return result;
}

std::string Procedure::getToothString() const
{
	switch (static_cast<Scope>(affectedTeeth.index()))
	{
		case Scope::AllOrNone:
			return std::string();
		case Scope::SingleTooth:
			return ToothUtils::getNomenclature(getToothIndex());
		case Scope::Range:
		{
			auto& [from, to] = std::get<ConstructionRange>(affectedTeeth);

			if (from == to) {
				return ToothUtils::getNomenclature(from, false);
			}

			return ToothUtils::getNomenclature(from, false) + "-" + ToothUtils::getNomenclature(to, false);
		}
	}

	return std::string();
}

