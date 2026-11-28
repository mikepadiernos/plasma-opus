// SPDX-License-Identifier: GPL-3.0-or-later

#include "archivewriter.h"
#include "archives.h"
#include "umaskfixture.h"

#include <K7Zip>
#include <KArchiveDirectory>
#include <KArchiveFile>
#include <KTar>
#include <KZip>
#include <KZipFileEntry>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include <archive.h>
#include <archive_entry.h>

#include <memory>

using ArchiveWriter::Format;

class ArchiveWriterTest : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;

    QString path(const QString &relative) const
    {
        return m_dir.filePath(relative);
    }

    static std::unique_ptr<KArchive> open(const QString &archivePath, Format format)
    {
        std::unique_ptr<KArchive> archive;
        if (format == Format::Zip || format == Format::ZipStored) {
            archive = std::make_unique<KZip>(archivePath);
        } else if (format == Format::SevenZip) {
            archive = std::make_unique<K7Zip>(archivePath);
        } else {
            archive = std::make_unique<KTar>(archivePath);
        }
        return archive->open(QIODevice::ReadOnly) ? std::move(archive) : nullptr;
    }

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(m_dir.isValid());
        QVERIFY(QDir(m_dir.path()).mkpath(QStringLiteral("photos/2026")));
        QFile photo(path(QStringLiteral("photos/2026/beach.txt")));
        QVERIFY(photo.open(QIODevice::WriteOnly));
        photo.write("sand");
        photo.close();
        QFile note(path(QStringLiteral("note.txt")));
        QVERIFY(note.open(QIODevice::WriteOnly));
        note.write("hello");
    }

    void suffixes()
    {
        QCOMPARE(ArchiveWriter::formats().size(), 6);
        QCOMPARE(ArchiveWriter::suffix(Format::Zip), QStringLiteral("zip"));
        QCOMPARE(ArchiveWriter::suffix(Format::ZipStored), QStringLiteral("zip"));
        QCOMPARE(ArchiveWriter::suffix(Format::TarGz), QStringLiteral("tar.gz"));
        QCOMPARE(ArchiveWriter::suffix(Format::TarXz), QStringLiteral("tar.xz"));
        QCOMPARE(ArchiveWriter::suffix(Format::TarZstd), QStringLiteral("tar.zst"));
        QCOMPARE(ArchiveWriter::suffix(Format::SevenZip), QStringLiteral("7z"));
        QCOMPARE(ArchiveWriter::firstVolumeName(QStringLiteral("photos.7z")), QStringLiteral("photos.7z.001"));
    }

    void sevenZipRejectsDestinationInsideSource()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("source")));
        const QString inside = root.filePath(QStringLiteral("source/out.7z"));
        QVERIFY(!ArchiveWriter::write(inside, Format::SevenZip, {root.filePath(QStringLiteral("source"))}).isEmpty());
        QVERIFY(!QFile::exists(inside));
    }

    void splitsIntoVolumesThatJoinBack()
    {
        const ScopedUmask scoped(027);
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QFile big(root.filePath(QStringLiteral("big.bin")));
        QVERIFY(big.open(QIODevice::WriteOnly));
        QByteArray data;
        for (int i = 0; i < 5000; ++i) {
            data.append(QByteArray::number(i * 7919));
        }
        big.write(data);
        big.close();
        const QString archivePath = root.filePath(QStringLiteral("big.zip"));
        QCOMPARE(ArchiveWriter::write(archivePath, Format::ZipStored, {big.fileName()}), QString());
        QFile whole(archivePath);
        QVERIFY(whole.open(QIODevice::ReadOnly));
        const QByteArray bytes = whole.readAll();
        whole.close();

        const ArchiveWriter::Volumes volumes = ArchiveWriter::splitIntoVolumes(archivePath, 4096);
        QCOMPARE(volumes.error, QString());
        QVERIFY(volumes.files.size() > 1);
        QCOMPARE(volumes.files.first(), root.filePath(ArchiveWriter::firstVolumeName(QStringLiteral("big.zip"))));
        QVERIFY(!QFile::exists(archivePath));
        QByteArray joined;
        for (const QString &volume : volumes.files) {
            QFile part(volume);
            QVERIFY(part.open(QIODevice::ReadOnly));
            QVERIFY(part.size() <= 4096);
            QCOMPARE(fileMode(volume), 0640);
            joined += part.readAll();
        }
        QCOMPARE(joined, bytes);
    }

    void failedSplitKeepsArchiveAndExistingVolumes()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString archivePath = root.filePath(QStringLiteral("note.7z"));
        QCOMPARE(ArchiveWriter::write(archivePath, Format::SevenZip, {path(QStringLiteral("note.txt"))}), QString());
        QFile existing(root.filePath(QStringLiteral("note.7z.002")));
        QVERIFY(existing.open(QIODevice::WriteOnly));
        existing.write("keep");
        existing.close();

        const ArchiveWriter::Volumes volumes = ArchiveWriter::splitIntoVolumes(archivePath, 16);
        QVERIFY(!volumes.error.isEmpty());
        QVERIFY(volumes.files.isEmpty());
        QVERIFY(QFile::exists(archivePath));
        QVERIFY(!QFile::exists(root.filePath(QStringLiteral("note.7z.001"))));
        QVERIFY(existing.open(QIODevice::ReadOnly));
        QCOMPARE(existing.readAll(), QByteArrayLiteral("keep"));
    }

    void writesEveryFormat_data()
    {
        QTest::addColumn<Format>("format");
        for (Format format : ArchiveWriter::formats()) {
            QTest::addRow("%s", qPrintable(ArchiveWriter::suffix(format) + QString::number(static_cast<int>(format)))) << format;
        }
    }

    void writesEveryFormat()
    {
        QFETCH(Format, format);
        const QString archivePath = path(QStringLiteral("out-%1.%2").arg(static_cast<int>(format)).arg(ArchiveWriter::suffix(format)));
        QCOMPARE(ArchiveWriter::write(archivePath, format, {path(QStringLiteral("photos")), path(QStringLiteral("note.txt"))}), QString());

        const std::unique_ptr<KArchive> archive = open(archivePath, format);
        QVERIFY(archive);
        const auto *beach = archive->directory()->file(QStringLiteral("photos/2026/beach.txt"));
        QVERIFY(beach);
        QCOMPARE(beach->data(), QByteArrayLiteral("sand"));
        const auto *note = archive->directory()->file(QStringLiteral("note.txt"));
        QVERIFY(note);
        QCOMPARE(note->data(), QByteArrayLiteral("hello"));
    }

    void storedZipDoesNotCompress()
    {
        const QString stored = path(QStringLiteral("stored.zip"));
        QCOMPARE(ArchiveWriter::write(stored, Format::ZipStored, {path(QStringLiteral("note.txt"))}), QString());
        KZip zip(stored);
        QVERIFY(zip.open(QIODevice::ReadOnly));
        const auto *note = static_cast<const KZipFileEntry *>(zip.directory()->file(QStringLiteral("note.txt")));
        QVERIFY(note);
        QCOMPARE(note->encoding(), 0);
    }

    void reportsErrors()
    {
        QVERIFY(!ArchiveWriter::write(path(QStringLiteral("missing/folder/out.zip")), Format::Zip, {path(QStringLiteral("note.txt"))}).isEmpty());
    }

    void encryptedZipRequiresPasswordAndExtractsSafely()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString zip = root.filePath(QStringLiteral("encrypted.zip"));
        QCOMPARE(
            ArchiveWriter::writeEncryptedZip(zip, Format::Zip, {path(QStringLiteral("photos")), path(QStringLiteral("note.txt"))}, QStringLiteral("secret")),
            QString());
        QString error;
        QCOMPARE(Archives::testIntegrity(zip, QString(), &error), Archives::TestResult::PasswordRequired);
        QCOMPARE(Archives::testIntegrity(zip, QStringLiteral("wrong"), &error), Archives::TestResult::Failed);
        QCOMPARE(Archives::testIntegrity(zip, QStringLiteral("secret"), &error), Archives::TestResult::Passed);
        QVERIFY(!ArchiveWriter::writeEncryptedZip(zip, Format::Zip, {path(QStringLiteral("note.txt"))}, QStringLiteral("secret")).isEmpty());
        QVERIFY(!Archives::extractProtected(zip, root.filePath(QStringLiteral("wrong")), QStringLiteral("wrong")).isEmpty());
        const QString target = root.filePath(QStringLiteral("extracted"));
        QCOMPARE(Archives::extractProtected(zip, target, QStringLiteral("secret")), QString());
        QFile note(QDir(target).filePath(QStringLiteral("note.txt")));
        QVERIFY(note.open(QIODevice::ReadOnly));
        QCOMPARE(note.readAll(), QByteArrayLiteral("hello"));
        QFile beach(QDir(target).filePath(QStringLiteral("photos/2026/beach.txt")));
        QVERIFY(beach.open(QIODevice::ReadOnly));
        QCOMPARE(beach.readAll(), QByteArrayLiteral("sand"));
        QVERIFY(!Archives::extractProtected(zip, target, QStringLiteral("secret")).isEmpty());
    }

    void encryptedStoredZipRoundTrips()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString zip = root.filePath(QStringLiteral("stored.zip"));
        QCOMPARE(ArchiveWriter::writeEncryptedZip(zip, Format::ZipStored, {path(QStringLiteral("note.txt"))}, QStringLiteral("secret")), QString());
        const QString target = root.filePath(QStringLiteral("output"));
        QCOMPARE(Archives::extractProtected(zip, target, QStringLiteral("secret")), QString());
        QFile note(QDir(target).filePath(QStringLiteral("note.txt")));
        QVERIFY(note.open(QIODevice::ReadOnly));
        QCOMPARE(note.readAll(), QByteArrayLiteral("hello"));
    }

    void encryptedZipRejectsUnsafeInput()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        QVERIFY(QDir(root.path()).mkpath(QStringLiteral("source/nested")));
        const QString source = root.filePath(QStringLiteral("source"));
        for (const QString &inside : {root.filePath(QStringLiteral("source/result.zip")), root.filePath(QStringLiteral("source/nested/result.zip"))}) {
            QVERIFY(!ArchiveWriter::writeEncryptedZip(inside, Format::Zip, {source}, QStringLiteral("secret")).isEmpty());
            QVERIFY(!QFile::exists(inside));
        }
        const QString output = root.filePath(QStringLiteral("out.zip"));
        const QString nullPassword = QStringLiteral("secret") + QChar(QChar::Null) + QStringLiteral("suffix");
        QVERIFY(ArchiveWriter::writeEncryptedZip(output, Format::Zip, {path(QStringLiteral("note.txt"))}, nullPassword).contains(QStringLiteral("NUL")));
        QVERIFY(!QFile::exists(output));
        QVERIFY(!ArchiveWriter::writeEncryptedZip(output, Format::Zip, {}, QStringLiteral("secret")).isEmpty());
        QVERIFY(!QFile::exists(output));
        QCOMPARE(ArchiveWriter::writeEncryptedZip(output, Format::Zip, {source}, QStringLiteral("secret")), QString());
        QVERIFY(Archives::extractProtected(output, root.filePath(QStringLiteral("extract")), nullPassword).contains(QStringLiteral("NUL")));
        QString error;
        QCOMPARE(Archives::testIntegrity(output, nullPassword, &error), Archives::TestResult::Failed);
        QVERIFY(error.contains(QStringLiteral("NUL")));
    }

    void protectedExtractionRejectsTraversal()
    {
        QTemporaryDir root;
        QVERIFY(root.isValid());
        const QString zip = root.filePath(QStringLiteral("unsafe.zip"));
        archive *writer = archive_write_new();
        QVERIFY(writer);
        QCOMPARE(archive_write_set_format_zip(writer), ARCHIVE_OK);
        const QByteArray path = QFile::encodeName(zip);
        QCOMPARE(archive_write_open_filename(writer, path.constData()), ARCHIVE_OK);
        archive_entry *entry = archive_entry_new();
        QVERIFY(entry);
        archive_entry_set_pathname(entry, "../escaped");
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0600);
        archive_entry_set_size(entry, 1);
        QCOMPARE(archive_write_header(writer, entry), ARCHIVE_OK);
        QCOMPARE(archive_write_data(writer, "x", 1), 1);
        archive_entry_free(entry);
        QCOMPARE(archive_write_close(writer), ARCHIVE_OK);
        QCOMPARE(archive_write_free(writer), ARCHIVE_OK);
        QVERIFY(!Archives::extractProtected(zip, root.filePath(QStringLiteral("out")), QStringLiteral("secret")).isEmpty());
        QVERIFY(!QFile::exists(root.filePath(QStringLiteral("escaped"))));
    }
};

QTEST_GUILESS_MAIN(ArchiveWriterTest)

#include "archivewritertest.moc"
