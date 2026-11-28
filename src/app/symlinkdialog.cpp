// SPDX-License-Identifier: GPL-3.0-or-later

#include "symlinkdialog.h"

#include "settings.h"
#include "symlinks.h"

#include <KLocalizedString>
#include <KUrlRequester>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

namespace
{
QString displayed(const QUrl &url)
{
    return url.toDisplayString(QUrl::PreferLocalFile);
}

// Keeps an ampersand in a path from becoming a mnemonic.
QString escaped(const QString &text)
{
    QString result = text;
    return result.replace(QLatin1Char('&'), QLatin1String("&&"));
}
}

SymlinkDialog::SymlinkDialog(const QUrl &target, const QString &targetName, const QUrl &otherFolder, QWidget *parent)
    : QDialog(parent)
    , m_target(target)
    , m_otherFolder(otherFolder)
    , m_name(new QLineEdit(Symlinks::defaultName(targetName, Settings::symlinkSuffix()), this))
    , m_besideTarget(new QRadioButton(i18nc("@option:radio", "Beside the target"), this))
    , m_otherPane(new QRadioButton(this))
    , m_chooseFolder(new QRadioButton(i18nc("@option:radio", "Another folder:"), this))
    , m_folder(new KUrlRequester(this))
    , m_relative(new QCheckBox(i18nc("@option:check", "Store a relative path to the target"), this))
    , m_preview(new QLabel(this))
{
    setObjectName(QStringLiteral("symlinkDialog"));
    setWindowTitle(i18nc("@title:window", "Create Symbolic Link"));
    auto *form = new QFormLayout(this);

    auto *targetLabel = new QLabel(displayed(target), this);
    targetLabel->setObjectName(QStringLiteral("symlinkTarget"));
    targetLabel->setTextFormat(Qt::PlainText);
    targetLabel->setWordWrap(true);
    targetLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(i18nc("@label", "Target:"), targetLabel);

    m_name->setObjectName(QStringLiteral("symlinkName"));
    form->addRow(i18nc("@label:textbox", "Link name:"), m_name);

    auto *folders = new QVBoxLayout;
    m_besideTarget->setObjectName(QStringLiteral("symlinkBesideTarget"));
    m_besideTarget->setToolTip(displayed(Symlinks::parentFolder(target)));
    m_besideTarget->setChecked(true);
    folders->addWidget(m_besideTarget);
    m_otherPane->setObjectName(QStringLiteral("symlinkOtherPane"));
    if (otherFolder.isEmpty()) {
        m_otherPane->setText(i18nc("@option:radio", "The other pane's folder"));
        m_otherPane->setEnabled(false);
        m_otherPane->setToolTip(i18nc("@info:tooltip", "Available while both panes are shown"));
    } else {
        m_otherPane->setText(escaped(i18nc("@option:radio", "The other pane's folder: %1", displayed(otherFolder))));
        const QString error = Symlinks::destinationError(target, otherFolder);
        m_otherPane->setEnabled(error.isEmpty());
        m_otherPane->setToolTip(error);
    }
    folders->addWidget(m_otherPane);
    m_chooseFolder->setObjectName(QStringLiteral("symlinkChooseFolder"));
    folders->addWidget(m_chooseFolder);
    m_folder->setObjectName(QStringLiteral("symlinkFolder"));
    m_folder->setMode(target.isLocalFile() ? KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly : KFile::Directory | KFile::ExistingOnly);
    m_folder->setUrl(Symlinks::parentFolder(target));
    m_folder->setEnabled(false);
    folders->addWidget(m_folder);
    form->addRow(i18nc("@label", "Create in:"), folders);

    m_relative->setObjectName(QStringLiteral("symlinkRelative"));
    m_relative->setToolTip(i18nc("@info:tooltip", "A relative link keeps working when the link and its target are moved together"));
    form->addRow(i18nc("@label", "Link target:"), m_relative);

    m_preview->setObjectName(QStringLiteral("symlinkPreview"));
    m_preview->setTextFormat(Qt::PlainText);
    m_preview->setWordWrap(true);
    m_preview->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(m_preview);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_create = buttons->button(QDialogButtonBox::Ok);
    m_create->setObjectName(QStringLiteral("symlinkCreate"));
    m_create->setText(i18nc("@action:button", "Create"));
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addRow(buttons);

    connect(m_name, &QLineEdit::textChanged, this, &SymlinkDialog::updatePlan);
    for (QRadioButton *button : {m_besideTarget, m_otherPane, m_chooseFolder}) {
        connect(button, &QRadioButton::toggled, this, &SymlinkDialog::updatePlan);
    }
    connect(m_chooseFolder, &QRadioButton::toggled, m_folder, &KUrlRequester::setEnabled);
    connect(m_folder, &KUrlRequester::textChanged, this, &SymlinkDialog::updatePlan);
    connect(m_relative, &QCheckBox::toggled, this, &SymlinkDialog::updatePlan);
    m_name->selectAll();
    m_name->setFocus();
    updatePlan();
}

QString SymlinkDialog::name() const
{
    return m_name->text();
}

QUrl SymlinkDialog::folder() const
{
    if (m_otherPane->isChecked()) {
        return m_otherFolder;
    }
    if (m_chooseFolder->isChecked()) {
        return m_folder->url();
    }
    return Symlinks::parentFolder(m_target);
}

bool SymlinkDialog::isRelative() const
{
    return m_relative->isChecked();
}

void SymlinkDialog::updatePlan()
{
    const Symlinks::Plan plan = Symlinks::plan(m_target, folder(), name(), isRelative() ? Symlinks::TargetForm::Relative : Symlinks::TargetForm::Absolute);
    m_preview->setText(plan.error.isEmpty() ? i18nc("@info", "The link will point to: %1", plan.target) : plan.error);
    m_create->setEnabled(plan.error.isEmpty());
}
