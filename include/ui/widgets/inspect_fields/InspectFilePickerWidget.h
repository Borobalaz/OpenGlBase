#pragma once

#include <functional>
#include <utility>

#include <QFileDialog>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "ui/widgets/inspect_fields/IInspectWidget.h"

class InspectFilePickerWidget : public QWidget, public IInspectWidget
{
public:
  explicit InspectFilePickerWidget(QWidget *parent = nullptr)
    : QWidget(parent)
  {
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    pathEdit = new QLineEdit(this);
    pathEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    browseButton = new QPushButton("...", this);
    browseButton->setFixedWidth(32);

    layout->addWidget(pathEdit, 1);
    layout->addWidget(browseButton, 0);

    QObject::connect(pathEdit, &QLineEdit::editingFinished, this, [this]()
    {
      if (valueChangedCallback)
      {
        valueChangedCallback(GetValue());
      }
    });

    QObject::connect(browseButton, &QPushButton::clicked, this, [this]()
    {
      const QString selectedPath = QFileDialog::getOpenFileName(
          this,
          dialogTitleValue.isEmpty() ? QStringLiteral("Select file") : dialogTitleValue,
          currentDirectory(),
          fileFilterValue);

      if (selectedPath.isEmpty())
      {
        return;
      }

      SetValue(selectedPath);
      if (valueChangedCallback)
      {
        valueChangedCallback(GetValue());
      }
    });
  }

  InspectFilePickerWidget(QString fieldId,
                         QString displayName,
                         QString groupName,
                         QString dialogTitle = QString(),
                         QString fileFilter = QStringLiteral("All files (*.*)"),
                         bool readOnly = false,
                         QWidget *parent = nullptr)
    : InspectFilePickerWidget(parent)
  {
    fieldIdValue = std::move(fieldId);
    displayNameValue = std::move(displayName);
    groupNameValue = std::move(groupName);
    dialogTitleValue = std::move(dialogTitle);
    fileFilterValue = std::move(fileFilter);
    readOnlyValue = readOnly;
    pathEdit->setReadOnly(readOnlyValue);
    browseButton->setEnabled(!readOnlyValue);
  }

  IInspectWidget *addToLayout(QHBoxLayout *layout) override
  {
    layout->addWidget(this, 1);
    return this;
  }

  QString fieldId() const override { return fieldIdValue; }
  QString displayName() const override { return displayNameValue; }
  QString groupName() const override { return groupNameValue; }
  bool isReadOnly() const override { return readOnlyValue; }

  void SetValue(const QVariant &value) override
  {
    const QSignalBlocker blocker(pathEdit);
    pathEdit->setText(value.toString());
  }

  QVariant GetValue() const override
  {
    return pathEdit->text();
  }

  std::function<void(const QVariant &)> valueChangedCallback;

private:
  QString currentDirectory() const
  {
    const QString text = pathEdit->text().trimmed();
    if (text.isEmpty())
    {
      return QString();
    }

    const QFileInfo info(text);
    return info.exists() ? info.absolutePath() : QString();
  }

  QString fieldIdValue;
  QString displayNameValue;
  QString groupNameValue;
  QString dialogTitleValue;
  QString fileFilterValue;
  bool readOnlyValue = false;
  QLineEdit *pathEdit = nullptr;
  QPushButton *browseButton = nullptr;
};