// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesplitter.h"

#include "filepublish.h"

#include <KLocalizedString>

#include <QFile>
#include <QFileInfo>

#include <algorithm>

namespace
{
constexpr qint64 BufferSize = 1024 * 1024;

bool openInput(QFile &file, FileSplitter::Result &result)
{
    if (!QFileInfo(file.fileName()).isFile()) {
        result.error = i18nc("@info", "Not a regular file: %1", file.fileName());
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = i18nc("@info", "Could not read %1: %2", file.fileName(), file.errorString());
        return false;
    }
    return true;
}

bool openOutput(QFile &file, FileSplitter::Result &result)
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly, QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
        result.error = i18nc("@info", "Could not create %1: %2", file.fileName(), file.errorString());
        return false;
    }
    result.outputs.append(file.fileName());
    return true;
}

bool writeBytes(QFile &file, const char *data, qint64 size, FileSplitter::Result &result)
{
    qint64 written = 0;
    while (written < size) {
        const qint64 count = file.write(data + written, size - written);
        if (count <= 0) {
            result.error = i18nc("@info", "Could not write %1: %2", file.fileName(), file.errorString());
            return false;
        }
        written += count;
    }
    return true;
}

bool finishOutput(QFile &file, FileSplitter::Result &result)
{
    if (!file.flush()) {
        result.error = i18nc("@info", "Could not finish writing %1: %2", file.fileName(), file.errorString());
        return false;
    }
    // Outputs are created owner-only and opened up only once complete, so incomplete ones stay private.
    if (const QString error = FilePublish::applyDefaultMode(file.handle()); !error.isEmpty()) {
        result.error = i18nc("@info", "Could not finish writing %1: %2", file.fileName(), error);
        return false;
    }
    file.close();
    return true;
}

bool isCanceled(const FileSplitter::CancelCheck &canceled, FileSplitter::Result &result)
{
    if (canceled && canceled()) {
        result.error = i18nc("@info", "Operation canceled; created files may be incomplete.");
        return true;
    }
    return false;
}
}

FileSplitter::Result FileSplitter::split(const QString &source, const QString &outputPrefix, qint64 partSize, const CancelCheck &canceled)
{
    Result result;
    if (outputPrefix.isEmpty() || partSize <= 0) {
        result.error = i18nc("@info", "A destination prefix and a positive part size are required.");
        return result;
    }
    QFile input(source);
    if (!openInput(input, result) || isCanceled(canceled, result)) {
        return result;
    }
    QByteArray buffer(BufferSize, Qt::Uninitialized);
    QFile output;
    qint64 remaining = 0;
    quint64 number = 0;
    while (true) {
        if (isCanceled(canceled, result)) {
            return result;
        }
        const qint64 requested = remaining > 0 ? std::min(BufferSize, remaining) : std::min(BufferSize, partSize);
        const qint64 count = input.read(buffer.data(), requested);
        if (count < 0) {
            result.error = i18nc("@info", "Could not read %1: %2", source, input.errorString());
            return result;
        }
        if (count == 0 && number > 0) {
            break;
        }
        if (remaining == 0) {
            output.setFileName(outputPrefix + QLatin1Char('.') + QString::number(++number).rightJustified(3, QLatin1Char('0')));
            if (!openOutput(output, result)) {
                return result;
            }
            remaining = partSize;
        }
        if (!writeBytes(output, buffer.constData(), count, result)) {
            return result;
        }
        remaining -= count;
        if (remaining == 0 || count == 0) {
            if (!finishOutput(output, result)) {
                return result;
            }
        }
        if (count == 0) {
            break;
        }
    }
    if (output.isOpen()) {
        finishOutput(output, result);
    }
    return result;
}

FileSplitter::Result FileSplitter::combine(const QStringList &parts, const QString &outputPath, const CancelCheck &canceled)
{
    Result result;
    if (parts.isEmpty() || outputPath.isEmpty()) {
        result.error = i18nc("@info", "Select input parts and an output file.");
        return result;
    }
    for (const QString &part : parts) {
        QFile input(part);
        if (!openInput(input, result)) {
            return result;
        }
    }
    if (isCanceled(canceled, result)) {
        return result;
    }
    QFile output(outputPath);
    if (!openOutput(output, result)) {
        return result;
    }
    QByteArray buffer(BufferSize, Qt::Uninitialized);
    for (const QString &part : parts) {
        QFile input(part);
        if (!openInput(input, result)) {
            return result;
        }
        while (true) {
            if (isCanceled(canceled, result)) {
                return result;
            }
            const qint64 count = input.read(buffer.data(), buffer.size());
            if (count < 0) {
                result.error = i18nc("@info", "Could not read %1: %2", part, input.errorString());
                return result;
            }
            if (count == 0) {
                break;
            }
            if (!writeBytes(output, buffer.constData(), count, result)) {
                return result;
            }
        }
    }
    finishOutput(output, result);
    return result;
}
