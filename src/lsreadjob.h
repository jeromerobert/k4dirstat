#pragma once
#include "kdirreadjob.h"

class QTextStream;
namespace KDirStat {
    class LsReadJob: public KObjDirReadJob {
        Q_OBJECT

    public:
        LsReadJob(KDirTree * tree, KDirInfo * parent, QTextStream & stream):
        KObjDirReadJob( tree, parent ), stream(stream){}
        virtual ~LsReadJob(){}
        virtual void read();
    private:
        QTextStream & stream;
    };
}

