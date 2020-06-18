#ifndef CHEATBINDER_H
#define CHEATBINDER_H

#include <QObject>
#include <Windows.h>

class CheatBinder : public QObject
{
    Q_OBJECT
public:
    explicit CheatBinder(QObject *parent = nullptr);

public slots:
    bool findProcess();

    void showProcessIsReady();

    bool enableAmmoHerbCheat();

    bool disableAmmoHerbCheat();

    bool enableHeldItem90Cheat();

    bool disableHeldItem90Cheat();

    bool enableAllItem90Cheat();

    bool disableAllItem90Cheat();

private:
    HANDLE hProc;

    /* zombie's retreat process */
    PBYTE pBase = NULL;
    DWORD baseSz;

    /* nw.dll */
    LPVOID pBaseOfNWDll = NULL;
    DWORD NWDllImageSz;
};

#endif // CHEATBINDER_H
