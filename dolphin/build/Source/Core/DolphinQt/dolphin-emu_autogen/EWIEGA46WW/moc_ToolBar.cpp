/****************************************************************************
** Meta object code from reading C++ file 'ToolBar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Source/Core/DolphinQt/ToolBar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ToolBar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ToolBar_t {
    QByteArrayData data[18];
    char stringdata0[242];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ToolBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ToolBar_t qt_meta_stringdata_ToolBar = {
    {
QT_MOC_LITERAL(0, 0, 7), // "ToolBar"
QT_MOC_LITERAL(1, 8, 11), // "OpenPressed"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 14), // "RefreshPressed"
QT_MOC_LITERAL(4, 36, 11), // "PlayPressed"
QT_MOC_LITERAL(5, 48, 12), // "PausePressed"
QT_MOC_LITERAL(6, 61, 11), // "StopPressed"
QT_MOC_LITERAL(7, 73, 17), // "FullScreenPressed"
QT_MOC_LITERAL(8, 91, 17), // "ScreenShotPressed"
QT_MOC_LITERAL(9, 109, 15), // "SettingsPressed"
QT_MOC_LITERAL(10, 125, 18), // "ControllersPressed"
QT_MOC_LITERAL(11, 144, 15), // "GraphicsPressed"
QT_MOC_LITERAL(12, 160, 11), // "StepPressed"
QT_MOC_LITERAL(13, 172, 15), // "StepOverPressed"
QT_MOC_LITERAL(14, 188, 14), // "StepOutPressed"
QT_MOC_LITERAL(15, 203, 11), // "SkipPressed"
QT_MOC_LITERAL(16, 215, 13), // "ShowPCPressed"
QT_MOC_LITERAL(17, 229, 12) // "SetPCPressed"

    },
    "ToolBar\0OpenPressed\0\0RefreshPressed\0"
    "PlayPressed\0PausePressed\0StopPressed\0"
    "FullScreenPressed\0ScreenShotPressed\0"
    "SettingsPressed\0ControllersPressed\0"
    "GraphicsPressed\0StepPressed\0StepOverPressed\0"
    "StepOutPressed\0SkipPressed\0ShowPCPressed\0"
    "SetPCPressed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ToolBar[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      16,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x06 /* Public */,
       3,    0,   95,    2, 0x06 /* Public */,
       4,    0,   96,    2, 0x06 /* Public */,
       5,    0,   97,    2, 0x06 /* Public */,
       6,    0,   98,    2, 0x06 /* Public */,
       7,    0,   99,    2, 0x06 /* Public */,
       8,    0,  100,    2, 0x06 /* Public */,
       9,    0,  101,    2, 0x06 /* Public */,
      10,    0,  102,    2, 0x06 /* Public */,
      11,    0,  103,    2, 0x06 /* Public */,
      12,    0,  104,    2, 0x06 /* Public */,
      13,    0,  105,    2, 0x06 /* Public */,
      14,    0,  106,    2, 0x06 /* Public */,
      15,    0,  107,    2, 0x06 /* Public */,
      16,    0,  108,    2, 0x06 /* Public */,
      17,    0,  109,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ToolBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ToolBar *_t = static_cast<ToolBar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OpenPressed(); break;
        case 1: _t->RefreshPressed(); break;
        case 2: _t->PlayPressed(); break;
        case 3: _t->PausePressed(); break;
        case 4: _t->StopPressed(); break;
        case 5: _t->FullScreenPressed(); break;
        case 6: _t->ScreenShotPressed(); break;
        case 7: _t->SettingsPressed(); break;
        case 8: _t->ControllersPressed(); break;
        case 9: _t->GraphicsPressed(); break;
        case 10: _t->StepPressed(); break;
        case 11: _t->StepOverPressed(); break;
        case 12: _t->StepOutPressed(); break;
        case 13: _t->SkipPressed(); break;
        case 14: _t->ShowPCPressed(); break;
        case 15: _t->SetPCPressed(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::OpenPressed)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::RefreshPressed)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::PlayPressed)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::PausePressed)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::StopPressed)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::FullScreenPressed)) {
                *result = 5;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::ScreenShotPressed)) {
                *result = 6;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::SettingsPressed)) {
                *result = 7;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::ControllersPressed)) {
                *result = 8;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::GraphicsPressed)) {
                *result = 9;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::StepPressed)) {
                *result = 10;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::StepOverPressed)) {
                *result = 11;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::StepOutPressed)) {
                *result = 12;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::SkipPressed)) {
                *result = 13;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::ShowPCPressed)) {
                *result = 14;
                return;
            }
        }
        {
            typedef void (ToolBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ToolBar::SetPCPressed)) {
                *result = 15;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject ToolBar::staticMetaObject = {
    { &QToolBar::staticMetaObject, qt_meta_stringdata_ToolBar.data,
      qt_meta_data_ToolBar,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *ToolBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ToolBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ToolBar.stringdata0))
        return static_cast<void*>(this);
    return QToolBar::qt_metacast(_clname);
}

int ToolBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void ToolBar::OpenPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ToolBar::RefreshPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ToolBar::PlayPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ToolBar::PausePressed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ToolBar::StopPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ToolBar::FullScreenPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void ToolBar::ScreenShotPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ToolBar::SettingsPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void ToolBar::ControllersPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void ToolBar::GraphicsPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void ToolBar::StepPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void ToolBar::StepOverPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void ToolBar::StepOutPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void ToolBar::SkipPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void ToolBar::ShowPCPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void ToolBar::SetPCPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
