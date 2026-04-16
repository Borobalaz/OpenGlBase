#include "ui/qt-adapters/InspectQtAdapter.h"

#include <sstream>

#include <QCoreApplication>
#include <QHash>
#include "ui/mediator/InspectProvider.h"

InspectQtAdapter::InspectQtAdapter(QObject* parent)
  : QObject(parent)
{
  syncTimer.setInterval(33);
  syncTimer.setTimerType(Qt::PreciseTimer);
  connect(&syncTimer, &QTimer::timeout, this, [this]()
  {
    SyncSnapshots();
  });
  syncTimer.start();
}

QStringList InspectQtAdapter::objectNames() const
{
  return providerNames;
}

int InspectQtAdapter::selectedObjectIndex() const
{
  return selectedIndex;
}

void InspectQtAdapter::setSelectedObjectIndex(int index)
{
  if (index == selectedIndex)
  {
    return;
  }

  if (index < 0 || index >= static_cast<int>(providers.size()))
  {
    selectedIndex = -1;
    currentFields.clear();
  }
  else
  {
    selectedIndex = index;
    currentFields = providers[static_cast<size_t>(selectedIndex)]->GetInspectFields();
  }

  fieldObjects.clear();
  fieldSnapshots.clear();
  for (const std::shared_ptr<InspectField>& field : currentFields)
  {
    if (!field)
    {
      continue;
    }

    if (QCoreApplication::instance() && field->thread() != QCoreApplication::instance()->thread())
    {
      field->moveToThread(QCoreApplication::instance()->thread());
    }

    fieldObjects.push_back(field.get());
    fieldSnapshots.insert(field->fieldId(), field->value());
  }

  emit selectedObjectIndexChanged();
  emit fieldsChanged();
  ++revision;
  emit fieldRevisionChanged();
}

QObjectList InspectQtAdapter::fields() const
{
  return fieldObjects;
}

int InspectQtAdapter::fieldRevision() const
{
  return revision;
}

void InspectQtAdapter::SetProviders(const std::vector<InspectProvider*>& newProviders)
{
  if (newProviders.size() == providers.size())
  {
    bool same = true;
    for (size_t i = 0; i < newProviders.size(); ++i)
    {
      if (newProviders[i] != providers[i])
      {
        same = false;
        break;
      }
    }

    if (same)
    {
      return;
    }
  }

  providers = newProviders;

  providerNames.clear();
  providerNames.reserve(static_cast<qsizetype>(providers.size()));
  for (InspectProvider* provider : providers)
  {
    providerNames.push_back(provider ? QString::fromStdString(provider->GetInspectDisplayName()) : QString());
  }

  emit objectNamesChanged();

  const int previousIndex = selectedIndex;
  if (providers.empty())
  {
    selectedIndex = -1;
    currentFields.clear();
  }
  else if (selectedIndex < 0 || selectedIndex >= static_cast<int>(providers.size()))
  {
    selectedIndex = 0;
    currentFields = providers.front()->GetInspectFields();
  }
  else
  {
    currentFields = providers[static_cast<size_t>(selectedIndex)]->GetInspectFields();
  }

  fieldObjects.clear();
  fieldSnapshots.clear();
  for (const std::shared_ptr<InspectField>& field : currentFields)
  {
    if (!field)
    {
      continue;
    }

    if (QCoreApplication::instance() && field->thread() != QCoreApplication::instance()->thread())
    {
      field->moveToThread(QCoreApplication::instance()->thread());
    }

    fieldObjects.push_back(field.get());
    fieldSnapshots.insert(field->fieldId(), field->value());
  }

  if (previousIndex != selectedIndex)
  {
    emit selectedObjectIndexChanged();
  }

  emit fieldsChanged();
  ++revision;
  emit fieldRevisionChanged();
}

QVariantMap InspectQtAdapter::fieldMeta(const QString& fieldId) const
{
  QVariantMap meta;

  const std::shared_ptr<InspectField> field = FindField(fieldId);
  if (field)
  {
    meta = field->meta();
  }

  return meta;
}

QVariant InspectQtAdapter::fieldValue(const QString& fieldId) const
{
  const std::shared_ptr<InspectField> field = FindField(fieldId);
  if (!field)
  {
    return QVariant();
  }

  return field->value();
}

bool InspectQtAdapter::setFieldValue(const QString& fieldId, const QVariant& value)
{
  const std::shared_ptr<InspectField> field = FindField(fieldId);
  if (!field)
  {
    return false;
  }

  field->setValue(value);
  fieldSnapshots[field->fieldId()] = field->value();
  ++revision;
  emit fieldRevisionChanged();
  return true;
}

void InspectQtAdapter::SyncSnapshots()
{
  for (const std::shared_ptr<InspectField>& field : currentFields)
  {
    if (!field)
    {
      continue;
    }

    const QVariant currentValue = field->value();
    const QVariant previousValue = fieldSnapshots.value(field->fieldId());
    if (currentValue != previousValue)
    {
      fieldSnapshots.insert(field->fieldId(), currentValue);
      field->notifyValueChanged();
      emit fieldValueChanged(field->fieldId(), currentValue);
      ++revision;
      emit fieldRevisionChanged();
    }
  }
}

std::shared_ptr<InspectField> InspectQtAdapter::FindField(const QString& fieldId) const
{
  for (const std::shared_ptr<InspectField>& field : currentFields)
  {
    if (field && field->fieldId() == fieldId)
    {
      return field;
    }
  }
  return nullptr;
}
