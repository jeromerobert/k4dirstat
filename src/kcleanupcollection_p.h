#include "kcleanup.h"
#include "kfileinfo.h"
#include <QAction>

namespace KDirStat {
class CleanupAction : public QAction {
  Q_OBJECT

public:
  CleanupAction(KCleanup cleanup, QObject * parent) : QAction(parent), cleanup_(cleanup) {
    connect(this, SIGNAL(triggered()), this, SLOT(slotTriggered()));
    setEnabled(false);
    refresh();
  }
  KCleanup &cleanup() { return cleanup_; }
  /** Refresh the action after it's cleanup changed */
  void refresh() { setText(cleanup_.title()); }

signals:
  /**
   * Emitted after the action is executed.
   *
   * Please note that there intentionally is no reference as to which
   * object the action was executed upon since this object very likely
   * doesn't exist any more.
   **/
  void executed();
public slots:
  void selectionChanged(KFileInfo *selection);
private slots:
  void slotTriggered() {
    cleanup_.execute(selection_);
    emit executed();
  }

private:
  KCleanup cleanup_;
  KFileInfo *selection_;
};
} // namespace KDirStat
