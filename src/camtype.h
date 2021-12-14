#ifndef CAMTYPE_H
#define CAMTYPE_H


enum class camtype {
  CS_673W,
  TV_IP651W,
  DCH_5030L,
};

#include <QStringList>
const QStringList CamList = {"CS_673W",
                             "TV_IP651W",
                             "DCH_5030L"};
/*
(QStringList()
                          << "CS_673W"
                          << "TV_IP651W"
                          << "DCH_5030L");
*/
#endif // CAMTYPE_H
