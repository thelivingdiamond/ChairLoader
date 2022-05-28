#pragma once
#include <QObject>
#include <QLineEdit>
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/PropertyRowNumberField.h"

class InplaceWidgetNumber : public QObject, public property_tree::InplaceWidget

{
	Q_OBJECT
public:
	inline InplaceWidgetNumber(PropertyRowNumberField* numberField, QPropertyTree* tree);

	~InplaceWidgetNumber(){
		if (entry_)
			entry_->setParent(0);
		entry_->deleteLater();
		entry_ = 0;
	}

	void commit();
	void* actualWidget() { return entry_; }
public slots:
	void onEditingFinished();
protected:
	QLineEdit* entry_;
	PropertyRowNumberField* row_;
	QPropertyTree* tree_;
};

InplaceWidgetNumber::InplaceWidgetNumber(PropertyRowNumberField* row, QPropertyTree* tree)
: InplaceWidget(tree)
, row_(row)
, entry_(new QLineEdit(tree))
, tree_(tree)
{
	entry_->setText(row_->valueAsString().c_str());
	connect(entry_, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

	entry_->selectAll();
}


inline void InplaceWidgetNumber::onEditingFinished()
{
	tree_->model()->rowAboutToBeChanged(row_);
	yasli::string str = entry_->text().toLocal8Bit().data();
	if(row_->setValueFromString(str.c_str()) || row_->multiValue())
		tree_->model()->rowChanged(row_);
	else
		tree_->_cancelWidget();
}

inline void InplaceWidgetNumber::commit()
{
	if(entry_)
		onEditingFinished();
}

