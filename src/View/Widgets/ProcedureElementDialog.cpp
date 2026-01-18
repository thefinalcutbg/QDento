#include "ProcedureElementDialog.h"
#include <array>

ProcedureElementDialog::ProcedureElementDialog(const std::set<std::string>& existing_codes, std::optional<ProcedureListElement> e)
    : 
    m_existing_codes(existing_codes),
    QDialog(nullptr)
{
    ui.setupUi(this);
	setWindowTitle(e ? tr("Edit Procedure") : tr("Add Procedure"));
    std::array<QString, (int)Procedure::Type::MaxCount> typeNames;
    typeNames[(int)Procedure::Type::General] = tr("General");
    typeNames[(int)Procedure::Type::ToothNonSpecific] = tr("Tooth Non-Specific");
    typeNames[(int)Procedure::Type::Depuratio] = tr("Full Debridement");
    typeNames[(int)Procedure::Type::DepuratioTooth] = tr("Debridement On Single Tooth");
    typeNames[(int)Procedure::Type::Restoration] = tr("Restoration");
    typeNames[(int)Procedure::Type::RemoveRestoration] = tr("Remove Restoration");
    typeNames[(int)Procedure::Type::Extraction] = tr("Extraction");
    typeNames[(int)Procedure::Type::Implant] = tr("Implant");
    typeNames[(int)Procedure::Type::Endodontic] = tr("Endodontic");
    typeNames[(int)Procedure::Type::Post] = tr("Post");
    typeNames[(int)Procedure::Type::RemovePost] = tr("Remove Post");
    typeNames[(int)Procedure::Type::PostCore] = tr("Post Core");
    typeNames[(int)Procedure::Type::PostCrown] = tr("Post Crown");
    typeNames[(int)Procedure::Type::Crown] = tr("Crown");
    typeNames[(int)Procedure::Type::Bridge] = tr("Bridge");
    typeNames[(int)Procedure::Type::RemoveCrownOrBridge] = tr("Remove Crown or Bridge");
    typeNames[(int)Procedure::Type::DenturePair] = tr("Denture Pair");
    typeNames[(int)Procedure::Type::Denture] = tr("Denture");
    typeNames[(int)Procedure::Type::Splint] = tr("Adhesive Bridge/Splint");
    typeNames[(int)Procedure::Type::MultipleExtraction] = tr("Multiple Extraction");

    for (auto& n : typeNames) {
        ui.typeCombo->addItem(n);
    }

    ui.codeEdit->setInputValidator(&not_empty_validator);
    ui.nameEdit->setInputValidator(&not_empty_validator);

    if (e) {
        ui.codeEdit->setText(e->code.c_str());
        ui.nameEdit->setText(e->name.c_str());
        ui.typeCombo->setCurrentIndex(static_cast<int>(e->type));
        ui.priceSpin->setValue(e->price);
        ui.diagnosisEdit->setText(e->diagnosis.c_str());
    
        m_existing_codes.erase(e->code);
    }

    connect(ui.okButton, &QPushButton::clicked, this, [&] {
        
        if (!ui.codeEdit->validateInput()) return;
        if (!ui.nameEdit->validateInput()) return;

        if (m_existing_codes.contains(ui.codeEdit->getText())) {
            ui.codeEdit->selectAll();
            ui.codeEdit->setFocus();
            return;
        }

        m_result = ProcedureListElement{
            .type = static_cast<Procedure::Type>(ui.typeCombo->currentIndex()),
            .code = ui.codeEdit->getText(),
            .name = ui.nameEdit->getText(),
            .price = ui.priceSpin->value(),
            .diagnosis = ui.diagnosisEdit->text().toStdString()
        };

        close();
   });

}

ProcedureElementDialog::~ProcedureElementDialog()
{}

