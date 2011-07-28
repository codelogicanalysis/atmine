#ifndef BROWSEDIALOG_H
#define BROWSEDIALOG_H

#include <QFileDialog>
#include <QString>

inline QString getFileName(QFileDialog ** browseFileDlg) {

	if (*browseFileDlg == NULL) {
		QString dir = QDir::currentPath();
		*browseFileDlg = new QFileDialog(NULL, QString("Open File"), dir, QString("All Files (*)"));
		(*browseFileDlg)->setOptions(QFileDialog::DontUseNativeDialog);
		(*browseFileDlg)->setFileMode(QFileDialog::ExistingFile);
		(*browseFileDlg)->setViewMode(QFileDialog::Detail);
	}
	if ((*browseFileDlg)->exec()){
		QStringList files = (*browseFileDlg)->selectedFiles();
		QString fileName = files[0];
		return fileName;
	}
	return "";
}

#endif // BROWSEDIALOG_H
