// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>
#include <QUrl>

class KUrlRequester;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

/**
 * Asks for the name, folder and target form of a new symbolic link. The link
 * is created beside its target by default; the other pane's folder or another
 * folder on the same host can be chosen instead.
 */
class SymlinkDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param target the item to link to
     * @param targetName the item's display name, used for the default link name
     * @param otherFolder the other pane's folder, or an empty URL when there is none
     */
    SymlinkDialog(const QUrl &target, const QString &targetName, const QUrl &otherFolder, QWidget *parent = nullptr);

    QString name() const;
    /// The chosen folder in which to create the link.
    QUrl folder() const;
    /// Whether the link stores a path relative to its folder.
    bool isRelative() const;

private:
    void updatePlan();

    QUrl m_target;
    QUrl m_otherFolder;
    QLineEdit *m_name;
    QRadioButton *m_besideTarget;
    QRadioButton *m_otherPane;
    QRadioButton *m_chooseFolder;
    KUrlRequester *m_folder;
    QCheckBox *m_relative;
    QLabel *m_preview;
    QPushButton *m_create;
};
