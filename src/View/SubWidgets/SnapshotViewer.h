#pragma once

#include <QWidget>
#include "ui_SnapshotViewer.h"
#include "View/Graphics/TeethViewScene.h"
#include "Model/Dental/Snapshot.h"

class SnapshotViewer : public QWidget
{
	Q_OBJECT

	TeethViewScene* teeth_scene;
	std::vector<Snapshot> m_snapshots;

	void paintEvent(QPaintEvent* e) override;

	void displaySnapshotToView(const Snapshot& h);

public:
	SnapshotViewer(QWidget *parent = nullptr);
	void setSnapshots(const std::vector<Snapshot>& snapshots);
	const Snapshot* getCurrentSnapshot() const;
	TeethViewScene* getTeethScene() { return teeth_scene; }
	bool isMostRecent() const;
	~SnapshotViewer();

private:
	Ui::SnapshotViewerClass ui;
};
