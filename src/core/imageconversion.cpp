// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageconversion.h"

#include "filepublish.h"
#include "imageediting.h"
#include "previewcontent.h"

#include <KLocalizedString>

#include <QDir>
#include <QFileInfo>
#include <QImageWriter>
#include <QSet>
#include <QTemporaryFile>

ImageConversion::Plan ImageConversion::plan(const QList<QUrl> &sources, const QString &directory, const QByteArray &format)
{
    Plan result;
    const QFileInfo dirInfo(directory);
    if (directory.isEmpty() || !dirInfo.isDir() || !dirInfo.isWritable()) {
        result.error = i18nc("@info", "Choose a writable destination folder.");
        return result;
    }
    if ((format != "png" && format != "jpeg" && format != "webp") || !QImageWriter::supportedImageFormats().contains(format)) {
        result.error = i18nc("@info", "The selected image format is not supported.");
        return result;
    }
    const QDir dir(dirInfo.absoluteFilePath());
    const QString extension = format == "jpeg" ? QStringLiteral("jpg") : QString::fromLatin1(format);
    QSet<QString> reserved;
    for (const QUrl &source : sources) {
        const QFileInfo sourceInfo(source.toLocalFile());
        if (!source.isLocalFile() || sourceInfo.fileName().isEmpty()) {
            result.entries.clear();
            result.error = i18nc("@info", "Only local image files can be converted.");
            return result;
        }
        const QString base = sourceInfo.completeBaseName();
        int suffix = 0;
        QString path;
        do {
            const QString name = base + (suffix ? QStringLiteral("-%1").arg(suffix) : QString()) + QLatin1Char('.') + extension;
            path = dir.filePath(name);
            ++suffix;
        } while (reserved.contains(path) || QFileInfo::exists(path) || QFileInfo(path).isSymLink());
        reserved.insert(path);
        result.entries.append({source, path});
    }
    return result;
}

QList<ImageConversion::Failure> ImageConversion::convert(const QList<Entry> &entries, const QByteArray &format, const QSize &bounds, int quality)
{
    QList<Failure> failures;
    for (const Entry &entry : entries) {
        const PreviewContent content = Preview::loadImage(entry.source, 0);
        if (content.kind != PreviewContent::Kind::Image) {
            const QString error = content.kind == PreviewContent::Kind::Error ? content.text : i18nc("@info", "Image cannot be loaded or is too large.");
            failures.append({entry, error});
            continue;
        }
        QImage image = content.image;
        if (bounds.isValid()) {
            image = ImageEditing::resize(image, image.size().scaled(bounds, Qt::KeepAspectRatio));
            if (image.isNull()) {
                failures.append({entry, i18nc("@info", "Could not resize the image.")});
                continue;
            }
        }
        QTemporaryFile temporary(QFileInfo(entry.destination).dir().filePath(QStringLiteral(".opus-conversion-XXXXXX")));
        if (!temporary.open()) {
            failures.append({entry, temporary.errorString()});
            continue;
        }
        QImageWriter writer(&temporary, format);
        if (format != "png") {
            writer.setQuality(quality);
        }
        if (!writer.write(image)) {
            failures.append({entry, writer.errorString()});
            continue;
        }
        if (!temporary.flush()) {
            failures.append({entry, temporary.errorString()});
            continue;
        }
        if (const QString error = FilePublish::applyDefaultMode(temporary.handle()); !error.isEmpty()) {
            failures.append({entry, error});
            continue;
        }
        const FilePublish::Publication published = FilePublish::publishNoReplace(temporary.handle(), temporary.fileName(), entry.destination);
        if (!published.error.isEmpty()) {
            failures.append({entry, published.error});
        } else {
            temporary.setAutoRemove(false);
        }
    }
    return failures;
}
