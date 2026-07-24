#include "xcalc.h"
#include "util.h"
#include "xcalcutil.h"
#include "xcalcrc.h"
#include "xcalcwindow.h"
#include <QMessageBox>
#include "qengine.h"
#include "aschar.h"
#include <QKeyEvent>
#include <QDialog>
#include <QAction>
#include <QPointer>

// Pushbutton layout, one list per mode (keyids)
idList DB,CB,HB,OB,BB; // DecimalButs, ComplexButs etc... (places buttons and links to actions)
idList DA,CA,HA,OA,BA; // Allowed ids for mode (no restrictions on size!)
idtextmap Keycaps; // single mapping from key id to key caption
idtextmap Keytips; // single mapping from key id to tooltip
idtextmap fname; // single mapping from func id to func name (used everywhere)
/* funid defined in xcalc.h */
const int NIDS = 1+FUN_LAST-FUN_NONE;

// Shortcuts

using namespace Qt;

// Undo text (lookup by funid!)

idseqmap xcshortcuts;

// Keys to handle locally (don't send to engine) mainly help functions and number input (not undoable)

idList local_funs;

bool XCALCWindow::m_mapsok = false;

int MaxInputDigits(RadixType rt,WordLength wl) {
    if (rt==rtDECIMAL) return DECMAXDIGITS;
    else if (rt==rtCOMPLEX) return CPLXMAXDIGITS;
    else if (rt==rtHEX) return
            wl==wl8BIT?HEXMAXDIGITS8:wl==wl16BIT?HEXMAXDIGITS16:wl==wl32BIT?HEXMAXDIGITS32:HEXMAXDIGITS64;
    else if (rt==rtOCTAL) return
            wl==wl8BIT?OCTMAXDIGITS8:wl==wl16BIT?OCTMAXDIGITS16:wl==wl32BIT?OCTMAXDIGITS32:OCTMAXDIGITS64;
    else if (rt==rtBINARY) return
            wl==wl8BIT?BINMAXDIGITS8:wl==wl16BIT?BINMAXDIGITS16:wl==wl32BIT?BINMAXDIGITS32:BINMAXDIGITS64;
    else return 42;
};

XCALCWindow::XCALCWindow(QWidget *parent):
    QMainWindow(parent),
    m_qengine(0),
    m_xengine(0),
    m_iState(0), // input key state (numbers, fractions)
    m_iState2(-4), // other part of key state when complex is being keyed
    m_fixtype(ftFIX),
    m_ang(atDEGREE) {
    // set help directory for everyone
    SetHelpDirDflt();
    // set icon
    QPixmap pm;
    QString pname=chext(appPath(),"gif");
    pm.load(pname);
    this->setWindowIcon(pm);
    // QSignalMapper removed: we use lambdas for action mapping
    if (!m_mapsok) InitMaps();
    m_qengine = ((XCalcApp*)qApp)->m_qengine;
    m_xengine = m_qengine->m_xengine;
    m_grid=0, m_lstk=0, m_but=0;

    SetupUi();

    setFocusPolicy(Qt::StrongFocus);
    // Modern connect syntax for engine signals
    if (m_qengine) {
        connect(m_qengine, &QEngine::changed, this, &XCALCWindow::updateslot);
        connect(m_qengine, &QEngine::radixchanged, this, &XCALCWindow::newradixslot);
    }
}

XCALCWindow::~XCALCWindow()
{
}

RadixType radix()
{
    return xcalc_radix;
}

void setradix(RadixType r)
{
    xcalc_radix = r;
}

WordLength wordLength() {
    return xcalc_wordLength;
}

void setWordLength(WordLength wl) {
    xcalc_wordLength = wl;
}

void XCALCWindow::InitMaps()
{
    // one time only init of action lists and maps:
    // Set up function names (used for messages & debugging)
    m_mapsok = true;
    // Set up shortcuts, one by one... (safer than array!)
    xcshortcuts[FUN_NONE] = 0;
    xcshortcuts[FUN_ABOUT]=CTRL+Key_A;

    xcshortcuts[FUN_0]=Key_0;
    xcshortcuts[FUN_1]=Key_1;
    xcshortcuts[FUN_2]=Key_2;
    xcshortcuts[FUN_3]=Key_3;
    xcshortcuts[FUN_4]=Key_4;
    xcshortcuts[FUN_5]=Key_5;
    xcshortcuts[FUN_6]=Key_6;
    xcshortcuts[FUN_7]=Key_7;
    xcshortcuts[FUN_8]=Key_8;
    xcshortcuts[FUN_9]=Key_9;
    xcshortcuts[FUN_A]=Key_A;
    xcshortcuts[FUN_B]=Key_B;
    xcshortcuts[FUN_C]=Key_C;
    xcshortcuts[FUN_D]=Key_D;
    xcshortcuts[FUN_E]=Key_E;
    xcshortcuts[FUN_F]=Key_F;
    xcshortcuts[FUN_POINT]=Key_Period;
    xcshortcuts[FUN_BACK]=Key_Backspace;
    xcshortcuts[FUN_IMAGPART]=Key_Comma;
    xcshortcuts[FUN_COPY]=Key_Copy;
    xcshortcuts[FUN_PASTE]=Key_Paste;
    xcshortcuts[FUN_FIX]=ALT+Key_I;
    xcshortcuts[FUN_SCI]=ALT+Key_S;
    xcshortcuts[FUN_ENG]=ALT+Key_E;
    xcshortcuts[FUN_FLOAT]=ALT+Key_F;
    xcshortcuts[FUN_FIX0]=ALT+Key_0;
    xcshortcuts[FUN_FIX1]=ALT+Key_1;
    xcshortcuts[FUN_FIX2]=ALT+Key_2;
    xcshortcuts[FUN_FIX3]=ALT+Key_3;
    xcshortcuts[FUN_FIX4]=ALT+Key_4;
    xcshortcuts[FUN_FIX5]=ALT+Key_5;
    xcshortcuts[FUN_FIX6]=ALT+Key_6;
    xcshortcuts[FUN_FIX7]=ALT+Key_7;
    xcshortcuts[FUN_FIX8]=ALT+Key_8;
    xcshortcuts[FUN_FIX9]=ALT+Key_9;
    xcshortcuts[FUN_FIXP]=0; // routed from rotr
    xcshortcuts[FUN_FIXM]=0; // routed from rotl
    xcshortcuts[FUN_DEG]=ALT+Key_G;
    xcshortcuts[FUN_RAD]=ALT+Key_R;
    xcshortcuts[FUN_BINARY]=ALT+Key_B;
    xcshortcuts[FUN_OCTAL]=ALT+Key_O;
    xcshortcuts[FUN_DECIMAL]=ALT+Key_D;
    xcshortcuts[FUN_COMPLEX]=ALT+Key_C;
    xcshortcuts[FUN_HEX]=ALT+Key_H;
    xcshortcuts[FUN_8BIT]=Key_F9;
    xcshortcuts[FUN_16BIT]=Key_F10;
    xcshortcuts[FUN_32BIT]=Key_F11;
    xcshortcuts[FUN_64BIT]=Key_F12;
    xcshortcuts[FUN_WLUP]=ALT+SHIFT+Key_W;
    xcshortcuts[FUN_WLDOWN]=ALT+Key_W;

    //General config
    xcshortcuts[FUN_CONFIG]=ALT+Key_N;
    xcshortcuts[FUN_EXIT]=ALT+Key_X;
    xcshortcuts[FUN_HELP]=Key_F1;
    xcshortcuts[FUN_KEYHELP]=CTRL+Key_F1;
    xcshortcuts[FUN_POPUP]=CTRL+SHIFT+Key_M;

    //Program keys

    xcshortcuts[FUN_PROG1]=META+Key_F1;
    xcshortcuts[FUN_PROG2]=META+Key_F2;
    xcshortcuts[FUN_PROG3]=META+Key_F3;
    xcshortcuts[FUN_PROG4]=META+Key_F4;
    xcshortcuts[FUN_PROG5]=META+Key_F5;
    xcshortcuts[FUN_PROG6]=META+Key_F6;
    xcshortcuts[FUN_PROG7]=META+Key_F7;
    xcshortcuts[FUN_PROG8]=META+Key_F8;
    xcshortcuts[FUN_PROG9]=META+Key_F9;
    xcshortcuts[FUN_PROG10]=META+Key_F10;
    xcshortcuts[FUN_PROG11]=META+Key_F11;
    xcshortcuts[FUN_PROG12]=META+Key_F12;
    xcshortcuts[FUN_EDITPR]=ALT+Key_F1;
    xcshortcuts[FUN_CHS]=Key_M;
    xcshortcuts[FUN_CLX]=0; // routed from BACK
    xcshortcuts[FUN_ENTER]=Key_Return;
    xcshortcuts[FUN_ADD]=Key_Plus;
    xcshortcuts[FUN_SUB]=Key_Minus;
    xcshortcuts[FUN_MUL]=Key_Asterisk;
    xcshortcuts[FUN_DIV]=Key_Slash;
    xcshortcuts[FUN_DIVF]=SHIFT+Key_D;
    xcshortcuts[FUN_MOD]=Key_Percent;
    xcshortcuts[FUN_MODF]=CTRL+Key_M;
    xcshortcuts[FUN_LASTX]=Key_L;
    xcshortcuts[FUN_UNDO]=CTRL+Key_Z;
    xcshortcuts[FUN_REDO]=CTRL+Key_Y;

    xcshortcuts[FUN_CLSTK]=SHIFT+Key_Z;
    xcshortcuts[FUN_TOIJ]=Key_I;
    xcshortcuts[FUN_FRAC]=Key_Space;
    xcshortcuts[FUN_DMS]=0; // routed from CONJ
    xcshortcuts[FUN_TODMS]=CTRL+Key_Apostrophe;
    xcshortcuts[FUN_ROUND]=Key_Dollar; // also s-R
    xcshortcuts[FUN_ROUND2]=SHIFT+Key_R; // translated to FUN_ROUND in translatekid
    xcshortcuts[FUN_XY]=Key_Less;
    xcshortcuts[FUN_RUP]=Key_Up;
    xcshortcuts[FUN_RDN]=Key_Down;
    xcshortcuts[FUN_PERC]=0; // manually routed from mod

    xcshortcuts[FUN_CONV]=Key_V;
    xcshortcuts[FUN_QCONV]=SHIFT+Key_V;
    xcshortcuts[FUN_CONS]=Key_K;
    xcshortcuts[FUN_QCONS]=SHIFT+Key_K;
    xcshortcuts[FUN_RAND]=Key_Question;
    xcshortcuts[FUN_TOFRAC]=CTRL+Key_F;
    xcshortcuts[FUN_TOINT]=CTRL+Key_I;
    xcshortcuts[FUN_CLEANFRAC]=SHIFT+Key_F;
    xcshortcuts[FUN_CART]=Key_Colon;
    xcshortcuts[FUN_POLAR]=Key_Semicolon;
    xcshortcuts[FUN_EXP]=SHIFT+Key_E;
    xcshortcuts[FUN_TEN]=SHIFT+Key_N;
    xcshortcuts[FUN_ROOT]=Key_R;
    xcshortcuts[FUN_SQRT]=Key_Q;
    xcshortcuts[FUN_QROOT]=Key_U;
    xcshortcuts[FUN_LN]=SHIFT+Key_L;
    xcshortcuts[FUN_LOG]=SHIFT+Key_G;
    xcshortcuts[FUN_POW]=Key_O;
    xcshortcuts[FUN_SQR]=SHIFT+Key_Q;
    xcshortcuts[FUN_CUBE]=SHIFT+Key_U;
    xcshortcuts[FUN_ABS]=0; // routed from OR
    xcshortcuts[FUN_SIN]=Key_S;
    xcshortcuts[FUN_COS]=0; // routed from "C"
    xcshortcuts[FUN_TAN]=Key_T;
    xcshortcuts[FUN_ASIN]=SHIFT+Key_S;
    xcshortcuts[FUN_ACOS]=SHIFT+Key_C;
    xcshortcuts[FUN_ATAN]=SHIFT+Key_T;
    xcshortcuts[FUN_SINH]=META+ALT+Key_S; // need two modifiers to not crash with predefined keys
    xcshortcuts[FUN_COSH]=META+ALT+Key_C;
    xcshortcuts[FUN_TANH]=META+ALT+Key_T;
    xcshortcuts[FUN_ARSINH]=META+ALT+SHIFT+Key_S;
    xcshortcuts[FUN_ARCOSH]=META+ALT+SHIFT+Key_C;
    xcshortcuts[FUN_ARTANH]=META+ALT+SHIFT+Key_T;
    xcshortcuts[FUN_RCP]=Key_Backslash;
    xcshortcuts[FUN_PI]=Key_P;
    xcshortcuts[FUN_FACT]=Key_Exclam;
    xcshortcuts[FUN_TOYX]=SHIFT+Key_I;
    xcshortcuts[FUN_REIM]=SHIFT+Key_J;
    xcshortcuts[FUN_CONJ]=Key_Apostrophe;
    xcshortcuts[FUN_AND]=Key_Ampersand;
    xcshortcuts[FUN_OR]=Key_Bar;
    xcshortcuts[FUN_XOR]=Key_X;
    xcshortcuts[FUN_NOT]=Key_AsciiTilde;
    xcshortcuts[FUN_NOT2]=Key_N; // translated to FUN_NOT in translatekid
    xcshortcuts[FUN_SHL]=SHIFT+Key_Left;
    xcshortcuts[FUN_SHR]=SHIFT+Key_Right;
    xcshortcuts[FUN_ASHL]=ALT+SHIFT+Key_Left;
    xcshortcuts[FUN_ASHR]=ALT+SHIFT+Key_Right;
    xcshortcuts[FUN_ROTL]=Key_Left;
    xcshortcuts[FUN_ROTR]=Key_Right;
    xcshortcuts[FUN_STO0]=CTRL+Key_0;
    xcshortcuts[FUN_STO1]=CTRL+Key_1;
    xcshortcuts[FUN_STO2]=CTRL+Key_2;
    xcshortcuts[FUN_STO3]=CTRL+Key_3;
    xcshortcuts[FUN_STO4]=CTRL+Key_4;
    xcshortcuts[FUN_STO5]=CTRL+Key_5;
    xcshortcuts[FUN_STO6]=CTRL+Key_6;
    xcshortcuts[FUN_STO7]=CTRL+Key_7;
    xcshortcuts[FUN_STO8]=CTRL+Key_8;
    xcshortcuts[FUN_STO9]=CTRL+Key_9;
    xcshortcuts[FUN_RCL0]=CTRL+SHIFT+Key_0;
    xcshortcuts[FUN_RCL1]=CTRL+SHIFT+Key_1;
    xcshortcuts[FUN_RCL2]=CTRL+SHIFT+Key_2;
    xcshortcuts[FUN_RCL3]=CTRL+SHIFT+Key_3;
    xcshortcuts[FUN_RCL4]=CTRL+SHIFT+Key_4;
    xcshortcuts[FUN_RCL5]=CTRL+SHIFT+Key_5;
    xcshortcuts[FUN_RCL6]=CTRL+SHIFT+Key_6;
    xcshortcuts[FUN_RCL7]=CTRL+SHIFT+Key_7;
    xcshortcuts[FUN_RCL8]=CTRL+SHIFT+Key_8;
    xcshortcuts[FUN_RCL9]=CTRL+SHIFT+Key_9;
    xcshortcuts[FUN_CLR0]=CTRL+ALT+Key_0;
    xcshortcuts[FUN_CLR1]=CTRL+ALT+Key_1;
    xcshortcuts[FUN_CLR2]=CTRL+ALT+Key_2;
    xcshortcuts[FUN_CLR3]=CTRL+ALT+Key_3;
    xcshortcuts[FUN_CLR4]=CTRL+ALT+Key_4;
    xcshortcuts[FUN_CLR5]=CTRL+ALT+Key_5;
    xcshortcuts[FUN_CLR6]=CTRL+ALT+Key_6;
    xcshortcuts[FUN_CLR7]=CTRL+ALT+Key_7;
    xcshortcuts[FUN_CLR8]=CTRL+ALT+Key_8;
    xcshortcuts[FUN_CLR9]=CTRL+ALT+Key_9;
    xcshortcuts[FUN_CLRMEM]=CTRL+ALT+Key_M;

    // Check shortcut ambiguity
    intlist il;
    for (funid kid=FUN_NONE;kid<FUN_LAST;kid=(funid)(kid+1)) {
        int sc = xcshortcuts[kid];
        if (sc!=0) {
            if (!il.contains(sc))
                il.append(sc);
            else {
                QString msg = QString("Duplicate:")+QString::number(sc,16);
                QMessageBox::warning(this,"Shortcuts",msg);
            }
        }
    }
    // create all actions, set accelerator and map triggered() instead of buttons and clicked() as was done in SetupUi
    // note: it would be better to make a radix dependent mapping instead of filtering manually in translatekid().
    for (funid kid=FUN_NONE;kid<FUN_LAST;kid=(funid)(kid+1)) {
        QAction *act=new QAction(Keycaps.value(kid),this);
        act->setShortcutContext(Qt::ApplicationShortcut);
        act->setShortcut(xcshortcuts.value(kid));
        // Modern connect: capture kid by value and call funcslot(int)
        connect(act, &QAction::triggered, this, [this, kid]() {
            this->funcslot(static_cast<int>(kid));
        });
        addAction(act); // action must be connected to a widget to work!
    }
    // Add pushbuttons depending on mode (captions automatically generated based on keyid)
    // Note: must be exactly BUTROWS*BUTCOLS buttons (currently 10*9)
    // Note: FUN_ENTER, FUN_UNDO and FUN_REDO automatically occupy two adjacent locations - make sure there is a FUN_NONE to the right
    // Decimal
    DB.clear();
    DB.append(FUN_STO0);    DB.append(FUN_STO1);    DB.append(FUN_RCL0);    DB.append(FUN_RCL1);    DB.append(FUN_CLR0);    DB.append(FUN_CLR1);    DB.append(FUN_CART);    DB.append(FUN_POLAR);   DB.append(FUN_CLRMEM);
    DB.append(FUN_SIN);     DB.append(FUN_COS);     DB.append(FUN_TAN);     DB.append(FUN_SINH);    DB.append(FUN_COSH);    DB.append(FUN_TANH);    DB.append(FUN_POW);     DB.append(FUN_UNDO);    DB.append(FUN_NONE);
    DB.append(FUN_ASIN);    DB.append(FUN_ACOS);    DB.append(FUN_ATAN);    DB.append(FUN_ARSINH);  DB.append(FUN_ARCOSH);  DB.append(FUN_ARTANH);  DB.append(FUN_ROOT);    DB.append(FUN_REDO);    DB.append(FUN_NONE);
    DB.append(FUN_BINARY);  DB.append(FUN_FIX4);    DB.append(FUN_FIX);     DB.append(FUN_SCI);     DB.append(FUN_ENG);     DB.append(FUN_CHS);     DB.append(FUN_LN  );    DB.append(FUN_DMS);     DB.append(FUN_TODMS);
    DB.append(FUN_OCTAL);   DB.append(FUN_FLOAT);   DB.append(FUN_SQRT);    DB.append(FUN_SQR);     DB.append(FUN_CUBE);    DB.append(FUN_ABS);     DB.append(FUN_EXP );    DB.append(FUN_FRAC);    DB.append(FUN_LASTX);
    DB.append(FUN_HEX);     DB.append(FUN_WLUP);    DB.append(FUN_7);       DB.append(FUN_8);       DB.append(FUN_9);       DB.append(FUN_ROUND);   DB.append(FUN_LOG );    DB.append(FUN_ADD);     DB.append(FUN_RUP);
    DB.append(FUN_COMPLEX); DB.append(FUN_WLDOWN);  DB.append(FUN_4);       DB.append(FUN_5);       DB.append(FUN_6);       DB.append(FUN_RAND);    DB.append(FUN_TEN );    DB.append(FUN_SUB);     DB.append(FUN_RDN);
    DB.append(FUN_DECIMAL); DB.append(FUN_EEX);     DB.append(FUN_1);       DB.append(FUN_2);       DB.append(FUN_3);       DB.append(FUN_PI);      DB.append(FUN_RCP );    DB.append(FUN_MUL);     DB.append(FUN_BACK);
    DB.append(FUN_DEG);     DB.append(FUN_RAD);     DB.append(FUN_POINT);   DB.append(FUN_0);       DB.append(FUN_IMAGPART);DB.append(FUN_PERC);    DB.append(FUN_FACT);    DB.append(FUN_DIV);     DB.append(FUN_CLSTK);
    DB.append(FUN_EXIT);    DB.append(FUN_ABOUT);   DB.append(FUN_CONFIG);  DB.append(FUN_HELP);    DB.append(FUN_KEYHELP); DB.append(FUN_XY);      DB.append(FUN_TOIJ);    DB.append(FUN_ENTER);   DB.append(FUN_NONE);
    // ... rest of InitMaps unchanged ...
}
