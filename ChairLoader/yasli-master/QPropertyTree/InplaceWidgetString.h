#pragma once
#include <QObject>
#include <QLineEdit>
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/PropertyRowString.h"

class InplaceWidgetString : public QObject, public property_tree::InplaceWidget
{
	Q_OBJECT
public:
  InplaceWidgetString(PropertyRowString* row, QPropertyTree* tree)
	: InplaceWidget(tree)
	, entry_(new QLineEdit(tree))
	, tree_(tree)
	, row_(row)
	{
#ifdef _MSC_VER
		initialValue_ = QString::fromUtf16((const ushort*)row->value().c_str());
#else
		initialValue_ = QString::fromWCharArray(row->value().c_str());
#endif
		entry_->setText(initialValue_);
		entry_->selectAll();
		QObject::connect(entry_.data(), SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
	}
	~InplaceWidgetString()
	{
		entry_->hide();
		entry_->setParent(0);
		entry_.take()->deleteLater();
	}

	void commit(){
		onEditingFinished();
	}
	void* actualWidget() { return entry_.data(); }

	public slots:
	void onEditingFinished(){
		if(initialValue_ != entry_->text() || row_->multiValue()){
			tree()->model()->rowAboutToBeChanged(row_);
			vector<wchar_t> str;
			QString text = entry_->text();
#ifdef _MSC_VER
			str.assign((const wchar_t*)text.utf16(), (const wchar_t*)(text.utf16() + text.size() + 1));
#else
			str.resize(text.size() + 1, L'\0');
			if (!text.isEmpty())
				text.toWCharArray(&str[0]);
#endif
			row_->setValue(&str[0]);
			tree()->model()->rowChanged(row_);
		}
		else
			tree_->_cancelWidget();
	}
protected:
    QPropertyTree* tree_;
	QScopedPointer<QLineEdit> entry_;
	PropertyRowString* row_;
    QString initialValue_;
};
