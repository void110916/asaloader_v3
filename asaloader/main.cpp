#include <QCoreApplication>
#include <QLocale>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTranslator>
int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
    const QString baseName = "asaloader_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      a.installTranslator(&translator);
      break;
    }
  }

  return a.exec();
}
