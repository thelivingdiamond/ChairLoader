#include <QSplitter>

class QPropertyTree;
class TwoTreesWidget : public QSplitter
{
	Q_OBJECT
public:
	TwoTreesWidget();

	QPropertyTree* outlineTree;

private slots:
	void onDataChanged();
	void onGenerate();
};
