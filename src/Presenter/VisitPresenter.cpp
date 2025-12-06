#include "VisitPresenter.h"

#include "GlobalSettings.h"
#include "Database/DbDentalVisit.h"
#include "Database/DbProcedure.h"
#include "Model/User.h"
#include "Model/FreeFunctions.h"
#include "Presenter/ProcedureDialogPresenter.h"
#include "Presenter/ProcedureEditorPresenter.h"
#include "Presenter/TabPresenter.h"
#include "Presenter/PatientHistoryPresenter.h"
#include "Presenter/DetailedStatusPresenter.h"
#include "View/Graphics/PaintHint.h"
#include "View/ModalDialogBuilder.h"
#include "View/Widgets/ProcedurePrintSelectDialog.h"
#include "View/Widgets/TabView.h"

VisitPresenter::VisitPresenter(TabView* tabView, std::shared_ptr<Patient> patient, long long rowId)
    :
    TabInstance(tabView, TabType::DentalVisit, patient),
    patient_info(tabView->listView()->tileInfo(), patient),
    view(tabView->listView()),
    m_visit(rowId ? DbDentalVisit::get(rowId) : DbDentalVisit::getNewDoc(patient->rowid))
{
    patient_info.setParent(this);

    surf_presenter.setStatusControl(this);

    if (m_visit.rowid) return;    
}

void VisitPresenter::statusChanged()
{
    m_checkModel = CheckModel(m_visit.teeth.getSelectedTeethPtr(m_selectedIndexes));
    m_dsnCheckModel = CheckModel(m_visit.teeth.getSelectedDsnPtr(m_selectedIndexes));

    view->setCheckModel(m_checkModel, m_dsnCheckModel);

    for (auto& idx : m_selectedIndexes)
    {
        view->repaintTooth(
            ToothPaintHint(m_visit.teeth[idx], patient->teethNotes[idx])
        );
    }

    if (m_selectedIndexes.size() == 1)
        surf_presenter.setTooth(m_visit.teeth[m_selectedIndexes[0]], patient->teethNotes[m_selectedIndexes[0]].size());

    makeEdited();
}

bool VisitPresenter::isValid()
{
    for (auto& p : m_visit.procedures)
    {
        auto& idx = p.getToothIndex();

        if (idx.supernumeral && !m_visit.teeth[idx.index][Dental::HasSupernumeral])
        {
            ModalDialogBuilder::showError(
                QObject::tr("To add procedure on a supernumeral tooth, add one in the dental chart first").toStdString()
            );
            return false;
        }

        return true;
    }
}

TabName VisitPresenter::getTabName()
{
    TabName n;

    if (m_visit.isNew()) {
        n.header += QObject::tr("New dental visit ").toStdString();
    }
    else {
        n.header += QObject::tr("Dental Visit ").toStdString();
        n.header += std::to_string(m_visit.number);
    }

    n.footer = patient->firstName;
    n.footer += " ";
    n.footer += patient->lastName;

    n.indicatorColor = patient->colorNameRgb;
    n.header_icon = CommonIcon::DENTALVISIT;
    return n;
}

long long VisitPresenter::rowID() const
{
    return m_visit.rowid;
}

bool VisitPresenter::save()
{
    if (!requiresSaving()) return true;

    if (!isValid()) return false;

    if (m_visit.isNew())
    {
        m_visit.rowid = DbDentalVisit::insert(m_visit, patient->rowid);
    }
    else
    {
        DbDentalVisit::update(m_visit);
    }

    edited = false;

    refreshTabName();

    return true;
}

bool VisitPresenter::isNew()
{
    return m_visit.isNew();
}

void VisitPresenter::setDataToView()
{
    view->setPresenter(this);
    
    patient_info.setDate(m_visit.date);

    patient_info.setCurrent(true);

    view->setDate(m_visit.date);

    view->setVisitNumber(m_visit.number);

    surf_presenter.setStatusControl(this);
    surf_presenter.setView(view->surfacePanel());
    view->surfacePanel()->setPresenter(&surf_presenter);

    for (int i = 0; i < 32; i++)
    {
        view->repaintTooth(ToothPaintHint(m_visit.teeth[i], patient->teethNotes[i]));
    }

    view->setNotes(patient->teethNotes);
    
    refreshProcedureView();

    view->setSelectedTeeth(m_selectedIndexes);

    view->focusTeethView();
}

void VisitPresenter::setAmbDate(const Date& date)
{
    m_visit.date = date;
    patient_info.setDate(date);
    makeEdited();
}

void VisitPresenter::setAmbNumber(int number)
{
    m_visit.number = number;
    makeEdited();
}

void VisitPresenter::setOther(int code)
{
    auto DO = [](Tooth& t) mutable
    {
        t.setStatus(Dental::Restoration, false);
        t.setSurface(Dental::Restoration, Dental::Distal, true);
        t.setSurface(Dental::Restoration, Dental::Occlusal, true);
    };

    auto MO = [](Tooth& t) mutable
    {
        t.setStatus(Dental::Restoration, false);
        t.setSurface(Dental::Restoration, Dental::Medial, true);
        t.setSurface(Dental::Restoration, Dental::Occlusal, true);
    };

    auto MOD = [](Tooth& t)
    {
        t.setStatus(Dental::Restoration, false);
        t.setSurface(Dental::Restoration, Dental::Distal, true);
        t.setSurface(Dental::Restoration, Dental::Occlusal, true);
        t.setSurface(Dental::Restoration, Dental::Medial, true);
    };

    auto& teeth = m_visit.teeth;

    for (auto idx : m_selectedIndexes)
    {

        auto& tooth = m_visit.teeth[idx];

        switch (code)
        {
            case OtherInputs::DO: DO(tooth); break;
            case OtherInputs::MO: MO(tooth); break;
            case OtherInputs::MOD: MOD(tooth); break;
            case OtherInputs::removeC: tooth.setStatus(Dental::Caries, false); break;
            case OtherInputs::removeO: tooth.setStatus(Dental::Restoration, false); break;
            case OtherInputs::removeBridge:
                teeth.removeBridgeOrSplint({ idx });
                break;
            case OtherInputs::removeAll:
                teeth.removeEveryStatus({ idx });
                break;
            case OtherInputs::removeDsn:
                tooth.setStatus(Dental::HasSupernumeral, false);
                break;
        }
    }

    for (int i = 0; i < 32; i++)
    {
        view->repaintTooth(ToothPaintHint{ m_visit.teeth[i], patient->teethNotes[i] });
    }

    statusChanged();
}

void VisitPresenter::setToothStatus(Dental::StatusType t, int code, bool supernumeral)
{
    //show multiple teeth warning message
    if (
        m_selectedIndexes.size() == 1 &&
        t == Dental::StatusType::General &&
        (code == Dental::Bridge || code == Dental::Splint) &&
        m_checkModel.generalStatus[code] == CheckState::unchecked
        )
    {
        ModalDialogBuilder::showMessage(QObject::tr("Select multiple teeth before adding this status").toStdString());
        return;
    }

    bool state{ false };

    auto& checkModel = supernumeral ? m_dsnCheckModel : m_checkModel;

    switch (t)
    {
        case Dental::StatusType::General: state = checkModel.generalStatus[code] != CheckState::checked; break;
        case Dental::StatusType::Restoration: state = checkModel.restorationStatus[code] != CheckState::checked; break;
        case Dental::StatusType::Caries: state = checkModel.cariesStatus[code] != CheckState::checked; break;
        case Dental::StatusType::DefectiveRestoration: state = checkModel.defRestoStatus[code] != CheckState::checked; break;
        case Dental::StatusType::NonCariesLesion: state = checkModel.nonCariesStatus[code] != CheckState::checked; break;
        case Dental::StatusType::Mobility: state = checkModel.mobilityStatus[code] != CheckState::checked; break;
    }

    m_visit.teeth.setStatus(m_selectedIndexes, t, code, state, supernumeral);

    if (t == Dental::StatusType::General)
    {
        if (code == Dental::Temporary) {
            refreshProcedureView(); //updates the teeth num
        }

        for (int i = 0; i < 32; i++) {
            view->repaintTooth(ToothPaintHint(m_visit.teeth[i], patient->teethNotes[i]));
        }
    }

    statusChanged();
}

void VisitPresenter::setSelectedTeeth(const std::vector<int>& SelectedIndexes)
{
    m_selectedIndexes = SelectedIndexes;

    m_checkModel = CheckModel(m_visit.teeth.getSelectedTeethPtr(SelectedIndexes));
    m_dsnCheckModel = CheckModel(m_visit.teeth.getSelectedDsnPtr(SelectedIndexes));

    view->setCheckModel(m_checkModel, m_dsnCheckModel);

    if(m_selectedIndexes.size() == 1){
        surf_presenter.setTooth(m_visit.teeth[m_selectedIndexes[0]], patient->teethNotes[m_selectedIndexes[0]].size());
        view->hideSurfacePanel(false);
    }
    else {
        view->hideSurfacePanel(true);
    }

    view->hideControlPanel(m_selectedIndexes.empty());

}

void VisitPresenter::historyRequested()
{
    PatientHistoryPresenter pr(*patient);

    pr.openDialog();

    view->setNotes(patient->teethNotes);
}

void VisitPresenter::openDetails(int toothIdx)
{
    if (toothIdx < 0 || toothIdx > 31) return;

    std::vector<Procedure> history = DbProcedure::getToothProcedures(patient->rowid, toothIdx);

    std::sort(history.begin(), history.end(), [](const Procedure& a, const Procedure& b) -> bool
        {
            return a.date < b.date;
        }
    );

    DetailedStatusPresenter d(toothIdx, patient->rowid, history);

    d.open();

    patient->teethNotes[toothIdx] = d.getNote();

    view->setNotes(patient->teethNotes);
    surf_presenter.setTooth(m_visit.teeth[m_selectedIndexes[0]], patient->teethNotes[m_selectedIndexes[0]].size());

}

void VisitPresenter::openDetails()
{
    if (m_selectedIndexes.size() != 1) {
        return;
    }
    
    openDetails(m_selectedIndexes[0]);
}

void VisitPresenter::refreshProcedureView()
{
    if (view == nullptr) return;

    m_visit.procedures.refreshTeethTemporary(m_visit.teeth);

    view->setProcedures(m_visit.procedures.list());
}

void VisitPresenter::addProcedure()
{
    //making a copy to apply the procedures before adding a new one
    DentalVisit copy = m_visit;
    for (auto& p : copy.procedures) {
        p.applyProcedure(copy.teeth);
    }

    ProcedureDialogPresenter p
    {
        copy,
        copy.teeth.getSelectedTeethPtr(m_selectedIndexes)
    };

    auto procedures = p.openDialog();

    if (procedures.empty()) return;

    m_visit.procedures.addProcedures(procedures);

    if (view == nullptr) return;

    refreshProcedureView();

    makeEdited();

}

void VisitPresenter::editProcedure(int index)
{
    if (index < 0 || index >= m_visit.procedures.size()) return;

    auto& m_for_edit = m_visit.procedures.at(index);

    ProcedureEditorPresenter p(m_for_edit);

    auto result = p.openDialog();

    if (!result) return;

    auto& m = result.value();

    m_visit.procedures.replaceProcedure(m, index);

    refreshProcedureView();

    makeEdited();

}

void VisitPresenter::deleteProcedure(int index)
{
    m_visit.procedures.removeProcedure(index);

    refreshProcedureView();

    makeEdited();
}

void VisitPresenter::moveProcedure(int from, int to)
{
    if(!m_visit.procedures.moveProcedure(from, to)) return;

    makeEdited();

    view->setProcedures(m_visit.procedures.list());
}

void VisitPresenter::showAppliedStatus()
{
    auto& procedures = m_visit.procedures.list();

    if (procedures.empty()) {
        ModalDialogBuilder::showMessage(
            QObject::tr("Add at least one procedure").toStdString()
        );
        return;
    }

    std::vector<Snapshot> result;

    result.reserve(procedures.size()+1);

    result.emplace_back();

    result.back().teeth = m_visit.teeth;
    result.back().date = m_visit.date;
    result.back().procedure_name = QObject::tr("Initial dental status").toStdString();

    for (auto& p : procedures)
    {
        result.emplace_back();

        auto& snapshot = result.back();

        snapshot.date = p.date;

        snapshot.affected_teeth = p.getArrayIndexes();
        snapshot.procedure_name = p.name;
        snapshot.procedure_diagnosis = p.diagnosis;
        snapshot.procedure_note = p.notes;
        snapshot.teeth = result[result.size()-2].teeth;

        p.applyProcedure(snapshot.teeth);
    }

    ModalDialogBuilder::showSnapshots(result);

}


void VisitPresenter::createInvoice()
{
    if (m_visit.procedures.empty()) {
        TabPresenter::get().openInvoice(patient->rowid, {});
        return;
    }

    auto selectedProcedures = ModalDialogBuilder::selectProcedures(m_visit.procedures.list());

    if (!selectedProcedures.has_value()) {
        return;
    }

    TabPresenter::get().openInvoice(patient->rowid, selectedProcedures.value());
}

void VisitPresenter::createPerioMeasurment()
{
    TabPresenter::get().openPerio(*this->patient.get());
}

VisitPresenter::~VisitPresenter()
{
    if (isCurrent()){
        view->setPresenter(nullptr);
    }
}
