#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QString>

/*
 * 全局界面样式（QSS）
 * ------------------------------------------------------------------
 * 统一在 main.cpp 里通过 qApp->setStyleSheet() 应用。
 * 风格：浅灰背景 + 白色卡片 + 淡边框 + 单一蓝色主色，克制干净。
 */
inline QString appStyleSheet()
{
    return QString::fromUtf8(R"QSS(
/* ---------- 基础 ---------- */
QWidget {
    font-family: "Microsoft YaHei", "微软雅黑";
    font-size: 13px;
    color: #1F2937;
}
QMainWindow, QDialog {
    background: #F4F6FA;
}
QWidget#Page {
    background: #F4F6FA;
}

/* ---------- 顶部工具栏 ---------- */
QToolBar#MainToolBar {
    background: #FFFFFF;
    border: none;
    border-bottom: 1px solid #E5E7EB;
    padding: 6px 10px;
    spacing: 6px;
}
QToolButton {
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 6px 12px;
    color: #4B5563;
}
QToolButton:hover {
    background: #EFF6FF;
    color: #2563EB;
}
QLabel#PageTitle {
    font-size: 15px;
    font-weight: bold;
    color: #111827;
}

/* ---------- 状态栏 ---------- */
QStatusBar {
    background: #FFFFFF;
    border-top: 1px solid #E5E7EB;
    color: #6B7280;
    padding-left: 8px;
}
QStatusBar::item { border: none; }

/* ---------- 卡片 ---------- */
QFrame#Card {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-radius: 10px;
}

/* ---------- 按钮 ---------- */
QPushButton {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    border-radius: 6px;
    padding: 6px 16px;
    min-height: 22px;
    color: #374151;
}
QPushButton:hover {
    border-color: #2563EB;
    color: #2563EB;
}
QPushButton:pressed {
    background: #EFF6FF;
}
QPushButton:disabled {
    color: #9CA3AF;
    border-color: #E5E7EB;
    background: #F9FAFB;
}
QPushButton#PrimaryButton {
    background: #2563EB;
    border: 1px solid #2563EB;
    color: #FFFFFF;
    font-weight: bold;
}
QPushButton#PrimaryButton:hover {
    background: #1D4ED8;
    border-color: #1D4ED8;
}
QPushButton#PrimaryButton:pressed {
    background: #1E40AF;
}
QPushButton#DangerButton {
    color: #DC2626;
    border-color: #FCA5A5;
}
QPushButton#DangerButton:hover {
    background: #FEF2F2;
    border-color: #DC2626;
}

/* 主页三个大按钮 */
QPushButton#HomeButton {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-radius: 12px;
    font-size: 16px;
    font-weight: bold;
    color: #1F2937;
    padding: 0px;
    /* 样式表里的 min-height 会覆盖 setFixedSize 的最小尺寸，
       所以三个大按钮的高度必须在这里显式写死，否则会被压扁 */
    min-width: 170px;
    max-width: 170px;
    min-height: 140px;
    max-height: 140px;
}
QPushButton#HomeButton:hover {
    background: #F8FBFF;
    border-color: #2563EB;
    color: #2563EB;
}
QPushButton#HomeButton:pressed {
    background: #EFF6FF;
}

/* ---------- 输入控件 ---------- */
QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox, QSpinBox {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    border-radius: 6px;
    padding: 5px 8px;
    min-height: 22px;
    selection-background-color: #2563EB;
    selection-color: #FFFFFF;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus,
QDoubleSpinBox:focus, QSpinBox:focus {
    border: 1px solid #2563EB;
}
QLineEdit:read-only {
    background: #F9FAFB;
    color: #6B7280;
}
QComboBox::drop-down, QDateEdit::drop-down {
    border: none;
    width: 20px;
}

/* ---------- 表格（Model/View） ---------- */
QTableView {
    background: #FFFFFF;
    alternate-background-color: #FAFBFC;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
    gridline-color: #EEF0F3;
    selection-background-color: #DBEAFE;
    selection-color: #1E3A8A;
}
QTableView::item {
    padding: 4px 6px;
}
QHeaderView::section {
    background: #F8FAFC;
    color: #4B5563;
    font-weight: bold;
    border: none;
    border-bottom: 1px solid #E5E7EB;
    border-right: 1px solid #EEF0F3;
    padding: 8px 6px;
}
QTableCornerButton::section {
    background: #F8FAFC;
    border: none;
}

/* ---------- 文字提示 ---------- */
QLabel#HintLabel {
    color: #6B7280;
}
QLabel#ErrorLabel {
    color: #DC2626;
}
QLabel#SuccessLabel {
    color: #059669;
}
)QSS");
}

#endif // APPSTYLE_H
