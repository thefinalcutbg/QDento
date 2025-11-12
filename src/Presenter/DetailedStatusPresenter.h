#pragma once

#include "Presenter/CheckModel.h"
#include "Model/Dental/Tooth.h"
#include "Model/Dental/Procedure.h"
#include <optional>

class DetailedStatus;

class DetailedStatusPresenter
{

	DetailedStatus* view{ nullptr };

	std::vector<Procedure> m_procedures;

	long long patientRowId;

	int m_tooth_index;
	std::string m_notes;

	std::optional<Tooth> _result{};

public:
	DetailedStatusPresenter(int toothIdx, long long patientRowId, const std::vector<Procedure>& toothProcedures);

	void setView(DetailedStatus* view);
	const std::string& getNote() const { return m_notes; }

	void okPressed();

	void open();
};

