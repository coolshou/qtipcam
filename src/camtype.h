#ifndef CAMTYPE_H
#define CAMTYPE_H


enum class camtype {
  CS_673W,
  TV_IP651W,
  DCS_5030L,
  DCS_930L
};

#include <QStringList>
const QStringList CamList = {"CS-673W",
                             "TV-IP651W",
                             "DCS-5030L",
                             "DCS-930L"};
/*
(QStringList()
                          << "CS_673W"
                          << "TV_IP651W"
                          << "DCS_5030L");
*/
#endif // CAMTYPE_H
