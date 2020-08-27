/********************************************************************************
** Form generated from reading UI file 'IBSU_SampleForQT.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IBSU_SAMPLEFORQT_H
#define UI_IBSU_SAMPLEFORQT_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_IBSU_SampleForQT
{
public:
    QWidget *centralWidget;
    QLabel *m_FrameImage;
    QGroupBox *groupBox;
    QComboBox *m_cboUsbDevices;
    QGroupBox *groupBox_2;
    QComboBox *m_cboCaptureSeq;
    QCheckBox *m_chkIgnoreFingerCount;
    QCheckBox *m_chkAutoCapture;
    QCheckBox *m_chkAutoContrast;
    QPushButton *m_btnCaptureStart;
    QPushButton *m_btnCaptureStop;
    QSlider *m_sliderContrast;
    QLabel *Lable_Contrast;
    QCheckBox *m_chkSaveImges;
    QTextEdit *m_txtContrast;
    QPushButton *m_btnImageFolder;
    QLabel *m_txtStatusMessage;
    QLabel *label;
    QGroupBox *groupBox_3;
    QLabel *label_2;
    QLabel *lblQuality_1;
    QLabel *lblQuality_2;
    QCheckBox *m_chkUseClearPlaten;
    QCheckBox *m_chkDrawSegmentImage;
    QCheckBox *m_chkNFIQScore;
    QLabel *m_txtNFIQScore;
    QCheckBox *m_chkInvalidArea;
    QCheckBox *m_chkDetectSmear;
    QComboBox *m_cboSmearLevel;
    QLabel *m_txtDllVersion;
    QLabel *m_txtCopyRight;
    QLabel *lblQuality_3;
    QLabel *lblQuality_4;

    void setupUi(QMainWindow *IBSU_SampleForQT)
    {
        if (IBSU_SampleForQT->objectName().isEmpty())
            IBSU_SampleForQT->setObjectName(QString::fromUtf8("IBSU_SampleForQT"));
        IBSU_SampleForQT->setEnabled(true);
        IBSU_SampleForQT->resize(772, 553);
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(9);
        IBSU_SampleForQT->setFont(font);
        IBSU_SampleForQT->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        IBSU_SampleForQT->setAnimated(true);
        centralWidget = new QWidget(IBSU_SampleForQT);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        m_FrameImage = new QLabel(centralWidget);
        m_FrameImage->setObjectName(QString::fromUtf8("m_FrameImage"));
        m_FrameImage->setGeometry(QRect(300, 80, 461, 431));
        m_FrameImage->setFont(font);
        m_FrameImage->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_FrameImage->setFrameShape(QFrame::Panel);
        m_FrameImage->setFrameShadow(QFrame::Sunken);
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 80, 281, 51));
        groupBox->setFont(font);
        groupBox->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_cboUsbDevices = new QComboBox(groupBox);
        m_cboUsbDevices->setObjectName(QString::fromUtf8("m_cboUsbDevices"));
        m_cboUsbDevices->setGeometry(QRect(10, 20, 261, 20));
        m_cboUsbDevices->setFont(font);
        m_cboUsbDevices->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 140, 281, 211));
        groupBox_2->setFont(font);
        groupBox_2->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_cboCaptureSeq = new QComboBox(groupBox_2);
        m_cboCaptureSeq->setObjectName(QString::fromUtf8("m_cboCaptureSeq"));
        m_cboCaptureSeq->setGeometry(QRect(10, 20, 261, 22));
        m_cboCaptureSeq->setFont(font);
        m_cboCaptureSeq->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkIgnoreFingerCount = new QCheckBox(groupBox_2);
        m_chkIgnoreFingerCount->setObjectName(QString::fromUtf8("m_chkIgnoreFingerCount"));
        m_chkIgnoreFingerCount->setGeometry(QRect(10, 90, 261, 17));
        m_chkIgnoreFingerCount->setFont(font);
        m_chkIgnoreFingerCount->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkIgnoreFingerCount->setChecked(false);
        m_chkAutoCapture = new QCheckBox(groupBox_2);
        m_chkAutoCapture->setObjectName(QString::fromUtf8("m_chkAutoCapture"));
        m_chkAutoCapture->setGeometry(QRect(10, 70, 261, 17));
        m_chkAutoCapture->setFont(font);
        m_chkAutoCapture->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkAutoCapture->setChecked(true);
        m_chkAutoContrast = new QCheckBox(groupBox_2);
        m_chkAutoContrast->setObjectName(QString::fromUtf8("m_chkAutoContrast"));
        m_chkAutoContrast->setGeometry(QRect(10, 50, 261, 17));
        m_chkAutoContrast->setFont(font);
        m_chkAutoContrast->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkAutoContrast->setChecked(true);
        m_btnCaptureStart = new QPushButton(groupBox_2);
        m_btnCaptureStart->setObjectName(QString::fromUtf8("m_btnCaptureStart"));
        m_btnCaptureStart->setGeometry(QRect(160, 150, 111, 21));
        m_btnCaptureStart->setFont(font);
        m_btnCaptureStart->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_btnCaptureStop = new QPushButton(groupBox_2);
        m_btnCaptureStop->setObjectName(QString::fromUtf8("m_btnCaptureStop"));
        m_btnCaptureStop->setGeometry(QRect(10, 150, 111, 21));
        m_btnCaptureStop->setFont(font);
        m_btnCaptureStop->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_sliderContrast = new QSlider(groupBox_2);
        m_sliderContrast->setObjectName(QString::fromUtf8("m_sliderContrast"));
        m_sliderContrast->setGeometry(QRect(60, 180, 161, 21));
        m_sliderContrast->setFont(font);
        m_sliderContrast->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_sliderContrast->setOrientation(Qt::Horizontal);
        Lable_Contrast = new QLabel(groupBox_2);
        Lable_Contrast->setObjectName(QString::fromUtf8("Lable_Contrast"));
        Lable_Contrast->setGeometry(QRect(10, 180, 51, 16));
        Lable_Contrast->setFont(font);
        Lable_Contrast->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkSaveImges = new QCheckBox(groupBox_2);
        m_chkSaveImges->setObjectName(QString::fromUtf8("m_chkSaveImges"));
        m_chkSaveImges->setEnabled(true);
        m_chkSaveImges->setGeometry(QRect(10, 120, 101, 20));
        m_chkSaveImges->setFont(font);
        m_chkSaveImges->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_txtContrast = new QTextEdit(groupBox_2);
        m_txtContrast->setObjectName(QString::fromUtf8("m_txtContrast"));
        m_txtContrast->setGeometry(QRect(230, 180, 41, 21));
        m_txtContrast->setFont(font);
        m_txtContrast->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_txtContrast->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_btnImageFolder = new QPushButton(groupBox_2);
        m_btnImageFolder->setObjectName(QString::fromUtf8("m_btnImageFolder"));
        m_btnImageFolder->setGeometry(QRect(110, 120, 31, 23));
        m_btnImageFolder->setFont(font);
        m_txtStatusMessage = new QLabel(centralWidget);
        m_txtStatusMessage->setObjectName(QString::fromUtf8("m_txtStatusMessage"));
        m_txtStatusMessage->setGeometry(QRect(10, 520, 751, 21));
        m_txtStatusMessage->setFont(font);
        m_txtStatusMessage->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_txtStatusMessage->setFrameShape(QFrame::Panel);
        m_txtStatusMessage->setFrameShadow(QFrame::Sunken);
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(0, 0, 781, 71));
        label->setFont(font);
        label->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        label->setPixmap(QPixmap(QString::fromUtf8(":/Images/IB_logo.bmp")));
        groupBox_3 = new QGroupBox(centralWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, 360, 131, 151));
        groupBox_3->setFont(font);
        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(8, 14, 121, 131));
        label_2->setFont(font);
        label_2->setPixmap(QPixmap(QString::fromUtf8(":/Images/Scanner.bmp")));
        lblQuality_1 = new QLabel(groupBox_3);
        lblQuality_1->setObjectName(QString::fromUtf8("lblQuality_1"));
        lblQuality_1->setGeometry(QRect(24, 47, 20, 71));
        lblQuality_1->setFont(font);
        lblQuality_1->setFrameShape(QFrame::NoFrame);
        lblQuality_2 = new QLabel(groupBox_3);
        lblQuality_2->setObjectName(QString::fromUtf8("lblQuality_2"));
        lblQuality_2->setGeometry(QRect(45, 47, 20, 71));
        lblQuality_2->setFont(font);
        lblQuality_2->setFrameShape(QFrame::NoFrame);
        m_chkUseClearPlaten = new QCheckBox(centralWidget);
        m_chkUseClearPlaten->setObjectName(QString::fromUtf8("m_chkUseClearPlaten"));
        m_chkUseClearPlaten->setGeometry(QRect(150, 370, 141, 16));
        m_chkUseClearPlaten->setFont(font);
        m_chkUseClearPlaten->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkUseClearPlaten->setChecked(true);
        m_chkDrawSegmentImage = new QCheckBox(centralWidget);
        m_chkDrawSegmentImage->setObjectName(QString::fromUtf8("m_chkDrawSegmentImage"));
        m_chkDrawSegmentImage->setGeometry(QRect(150, 390, 141, 31));
        m_chkDrawSegmentImage->setFont(font);
        m_chkDrawSegmentImage->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkDrawSegmentImage->setChecked(true);
        m_chkNFIQScore = new QCheckBox(centralWidget);
        m_chkNFIQScore->setObjectName(QString::fromUtf8("m_chkNFIQScore"));
        m_chkNFIQScore->setGeometry(QRect(150, 430, 51, 16));
        m_chkNFIQScore->setFont(font);
        m_chkNFIQScore->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkNFIQScore->setChecked(true);
        m_txtNFIQScore = new QLabel(centralWidget);
        m_txtNFIQScore->setObjectName(QString::fromUtf8("m_txtNFIQScore"));
        m_txtNFIQScore->setGeometry(QRect(220, 430, 61, 21));
        m_txtNFIQScore->setFont(font);
        m_txtNFIQScore->setFrameShape(QFrame::Panel);
        m_txtNFIQScore->setFrameShadow(QFrame::Sunken);
        m_chkInvalidArea = new QCheckBox(centralWidget);
        m_chkInvalidArea->setObjectName(QString::fromUtf8("m_chkInvalidArea"));
        m_chkInvalidArea->setEnabled(false);
        m_chkInvalidArea->setGeometry(QRect(150, 460, 131, 16));
        m_chkInvalidArea->setFont(font);
        m_chkInvalidArea->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkInvalidArea->setChecked(false);
        m_chkDetectSmear = new QCheckBox(centralWidget);
        m_chkDetectSmear->setObjectName(QString::fromUtf8("m_chkDetectSmear"));
        m_chkDetectSmear->setGeometry(QRect(150, 480, 101, 16));
        m_chkDetectSmear->setFont(font);
        m_chkDetectSmear->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_chkDetectSmear->setChecked(true);
        m_cboSmearLevel = new QComboBox(centralWidget);
        m_cboSmearLevel->setObjectName(QString::fromUtf8("m_cboSmearLevel"));
        m_cboSmearLevel->setGeometry(QRect(240, 480, 51, 20));
        m_cboSmearLevel->setFont(font);
        m_cboSmearLevel->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        m_txtDllVersion = new QLabel(centralWidget);
        m_txtDllVersion->setObjectName(QString::fromUtf8("m_txtDllVersion"));
        m_txtDllVersion->setGeometry(QRect(440, 25, 321, 21));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Arial"));
        font1.setPointSize(9);
        font1.setBold(true);
        font1.setWeight(75);
        m_txtDllVersion->setFont(font1);
        m_txtCopyRight = new QLabel(centralWidget);
        m_txtCopyRight->setObjectName(QString::fromUtf8("m_txtCopyRight"));
        m_txtCopyRight->setGeometry(QRect(440, 50, 321, 16));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Arial"));
        font2.setPointSize(10);
        m_txtCopyRight->setFont(font2);
        lblQuality_3 = new QLabel(centralWidget);
        lblQuality_3->setObjectName(QString::fromUtf8("lblQuality_3"));
        lblQuality_3->setGeometry(QRect(76, 407, 20, 71));
        lblQuality_3->setFont(font);
        lblQuality_3->setFrameShape(QFrame::NoFrame);
        lblQuality_4 = new QLabel(centralWidget);
        lblQuality_4->setObjectName(QString::fromUtf8("lblQuality_4"));
        lblQuality_4->setGeometry(QRect(97, 407, 20, 71));
        lblQuality_4->setFont(font);
        lblQuality_4->setFrameShape(QFrame::NoFrame);
        IBSU_SampleForQT->setCentralWidget(centralWidget);

        retranslateUi(IBSU_SampleForQT);

        QMetaObject::connectSlotsByName(IBSU_SampleForQT);
    } // setupUi

    void retranslateUi(QMainWindow *IBSU_SampleForQT)
    {
        IBSU_SampleForQT->setWindowTitle(QApplication::translate("IBSU_SampleForQT", "IntegrationSample for QT", 0, QApplication::UnicodeUTF8));
        m_FrameImage->setText(QString());
        groupBox->setTitle(QApplication::translate("IBSU_SampleForQT", "Devices", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("IBSU_SampleForQT", "Fingerprint Capture", 0, QApplication::UnicodeUTF8));
        m_chkIgnoreFingerCount->setText(QApplication::translate("IBSU_SampleForQT", "Trigger invalid finger count on auto-capture Mode", 0, QApplication::UnicodeUTF8));
        m_chkAutoCapture->setText(QApplication::translate("IBSU_SampleForQT", "Automatic Capture for Fingerprints", 0, QApplication::UnicodeUTF8));
        m_chkAutoContrast->setText(QApplication::translate("IBSU_SampleForQT", "Automatic Contrast Optimizaiton", 0, QApplication::UnicodeUTF8));
        m_btnCaptureStart->setText(QApplication::translate("IBSU_SampleForQT", "Start", 0, QApplication::UnicodeUTF8));
        m_btnCaptureStop->setText(QApplication::translate("IBSU_SampleForQT", "Stop", 0, QApplication::UnicodeUTF8));
        Lable_Contrast->setText(QApplication::translate("IBSU_SampleForQT", "Contrast", 0, QApplication::UnicodeUTF8));
        m_chkSaveImges->setText(QApplication::translate("IBSU_SampleForQT", "Save Images", 0, QApplication::UnicodeUTF8));
        m_btnImageFolder->setText(QApplication::translate("IBSU_SampleForQT", "...", 0, QApplication::UnicodeUTF8));
        m_txtStatusMessage->setText(QApplication::translate("IBSU_SampleForQT", "Ready", 0, QApplication::UnicodeUTF8));
        label->setText(QString());
        groupBox_3->setTitle(QApplication::translate("IBSU_SampleForQT", "Device Quality", 0, QApplication::UnicodeUTF8));
        label_2->setText(QString());
        lblQuality_1->setText(QString());
        lblQuality_2->setText(QString());
        m_chkUseClearPlaten->setText(QApplication::translate("IBSU_SampleForQT", "Detect clear platen", 0, QApplication::UnicodeUTF8));
        m_chkDrawSegmentImage->setText(QApplication::translate("IBSU_SampleForQT", "Draw quadrangle for \n"
"segment image", 0, QApplication::UnicodeUTF8));
        m_chkNFIQScore->setText(QApplication::translate("IBSU_SampleForQT", "NFIQ", 0, QApplication::UnicodeUTF8));
        m_txtNFIQScore->setText(QString());
        m_chkInvalidArea->setText(QApplication::translate("IBSU_SampleForQT", "Invalid area", 0, QApplication::UnicodeUTF8));
        m_chkDetectSmear->setText(QApplication::translate("IBSU_SampleForQT", "Detect smear", 0, QApplication::UnicodeUTF8));
        m_txtDllVersion->setText(QApplication::translate("IBSU_SampleForQT", "<html><head/><body><p><span style=\" font-size:14pt; color:#ffffff;\">TextLabel</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        m_txtCopyRight->setText(QApplication::translate("IBSU_SampleForQT", "<html><head/><body><p><span style=\" font-size:10pt; color:#ffffff;\">Copyright (c) Integrated Biometrics</span></p></body></html>", 0, QApplication::UnicodeUTF8));
        lblQuality_3->setText(QString());
        lblQuality_4->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class IBSU_SampleForQT: public Ui_IBSU_SampleForQT {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IBSU_SAMPLEFORQT_H
