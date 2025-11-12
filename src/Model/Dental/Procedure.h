#pragma once
#include <string>
#include <array>
#include <variant>
#include <optional>
#include <vector>
#include "Model/Date.h"

#include "ToothIndex.h"

class ToothContainer;
class Tooth;

struct ConstructionRange {

    int toothFrom{ -1 };
    int toothTo{ -1 };

    ConstructionRange(int from, int to) : toothFrom{ from }, toothTo{ to }{
        if (toothTo < toothFrom) std::swap(toothTo, toothFrom);
    }

    int getTeethCount() const { return toothTo - toothFrom + 1; }
    bool isFromSameJaw() const { return (toothFrom < 16) == (toothTo < 16); }
    bool isNotSingleRange() const { return toothFrom != toothTo; }
};

struct RestorationData
{
    std::array<bool, 6>surfaces{ 0,0,0,0,0,0 };
    bool post{ false };

    bool isValid() const {
         return std::find(surfaces.begin(), surfaces.end(), true) != surfaces.end();
    }
};

typedef std::variant<
            std::monostate, 
            RestorationData
            > AdditionalParameters;

typedef std::variant<
    std::monostate,
    ToothIndex,
    ConstructionRange
    > AffectedTeeth;

struct Procedure
{
    enum class Scope {
        AllOrNone,
        SingleTooth,
        Range
    };

    enum class Type
    {
        General = 0,
        ToothNonSpecific,
        Depuratio,
        DepuratioTooth,
        Restoration,
        RemoveRestoration,
        Extraction,
        Implant,
        Endodontic,
        Post,
        RemovePost,
        PostCore,
        PostCrown,
        Crown,
        Bridge,
        RemoveCrownOrBridge,
        DenturePair,
        Denture,
        Splint,
        MultipleExtraction,
        MaxCount
    };

    std::string code;
    std::string name;

    long long dentist_rowid;

    Type type;

    std::string diagnosis;

    double price;

    std::string notes;

    AffectedTeeth affectedTeeth;

    AdditionalParameters param;

    Date date;

    std::vector<const Tooth*> applyProcedure(ToothContainer& teeth) const;
    static Scope getScope(Type type);
    Scope getScope() const { return getScope(type); };
    //Can return invalid index with -1 value
    const ToothIndex& getToothIndex() const;
    bool hasSurfaceData() const { return type == Type::Restoration || type == Type::RemoveRestoration; }
    bool affectsToothIdx(int toothIdx) const;
    std::vector<int> getArrayIndexes() const;
    std::string getToothString() const;
};

