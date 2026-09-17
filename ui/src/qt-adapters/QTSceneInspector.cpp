#include "ui/qt-adapters/QTSceneInspector.h"

#include <limits>
#include <sstream>

#include <QCoreApplication>
#include <QHash>
#include "ui/widgets/inspect_fields/InspectColorFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectNumberFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectTextFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectVec3FieldWidget.h"
#include "ui/widgets/inspect_fields/InspectProvider.h"
#include "ui/widgets/inspect_fields/IInspectWidget.h"
#include "ui/widgets/inspect_fields/InspectCheckboxFieldWidget.h"

namespace
{
QVariant ToVariant(const InspectValue& value)
{
  if (const auto* boolean = std::get_if<bool>(&value)) return *boolean;
  if (const auto* integer = std::get_if<int>(&value)) return *integer;
  if (const auto* number = std::get_if<double>(&value)) return *number;
  if (const auto* text = std::get_if<std::string>(&value)) return QString::fromStdString(*text);
  if (const auto* vector = std::get_if<glm::vec3>(&value)) return QVariantList{vector->x, vector->y, vector->z};
  return {};
}

InspectValue ToInspectValue(const QVariant& value, InspectFieldType type)
{
  switch (type)
  {
    case InspectFieldType::Boolean: return value.toBool();
    case InspectFieldType::Number: return value.toDouble();
    case InspectFieldType::Color:
    case InspectFieldType::Vector3:
    {
      const QVariantList list = value.toList();
      if (list.size() >= 3)
        return glm::vec3(static_cast<float>(list[0].toDouble()), static_cast<float>(list[1].toDouble()), static_cast<float>(list[2].toDouble()));
      return glm::vec3(0.0f);
    }
    default: return value.toString().toStdString();
  }
}

class QtInspectFieldAdapter : public QObject, public IInspectWidget
{
public:
  explicit QtInspectFieldAdapter(InspectFieldPtr engineField)
    : field(std::move(engineField))
  {
    const QString id = QString::fromStdString(field->id);
    const QString name = QString::fromStdString(field->displayName);
    const QString group = QString::fromStdString(field->groupName);

    switch (field->type)
    {
      case InspectFieldType::Boolean:
      {
        auto checkbox = std::make_shared<InspectCheckboxFieldWidget>(id, name, group, field->readOnly);
        checkbox->valueChangedCallback = [this](const QVariant& value) { field->SetValue(ToInspectValue(value, field->type)); };
        widget = std::move(checkbox);
        break;
      }
      case InspectFieldType::Number:
      {
        auto number = std::make_shared<InspectNumberFieldWidget>(id, name, group,
          [this]() { return std::get<double>(field->GetValue()); },
          [this](double value) { field->SetValue(value); },
          field->minimum, field->maximum, field->step, field->readOnly);
        widget = std::move(number);
        break;
      }
      case InspectFieldType::Color:
      {
        auto color = std::make_shared<InspectColorFieldWidget>(id, name, group, field->readOnly);
        color->valueChangedCallback = [this](const QVariant& value) { field->SetValue(ToInspectValue(value, field->type)); };
        widget = std::move(color);
        break;
      }
      case InspectFieldType::Vector3:
      {
        auto vector = std::make_shared<InspectVec3FieldWidget>(id, name, group, field->readOnly);
        vector->valueChangedCallback = [this](const QVariant& value) { field->SetValue(ToInspectValue(value, field->type)); };
        widget = std::move(vector);
        break;
      }
      default:
      {
        auto text = std::make_shared<InspectTextFieldWidget>(id, name, group, field->readOnly);
        text->valueChangedCallback = [this](const QVariant& value) { field->SetValue(ToInspectValue(value, field->type)); };
        widget = std::move(text);
        break;
      }
    }

    widget->SetValue(ToVariant(field->GetValue()));
  }

  QString fieldId() const override { return QString::fromStdString(field->id); }
  QString displayName() const override { return QString::fromStdString(field->displayName); }
  QString groupName() const override { return QString::fromStdString(field->groupName); }
  bool isReadOnly() const override { return field->readOnly; }
  double minimum() const override { return field->minimum; }
  double maximum() const override { return field->maximum; }
  IInspectWidget* addToLayout(QHBoxLayout* layout) override { return widget->addToLayout(layout); }
  void SetValue(const QVariant& value) override
  {
    field->SetValue(ToInspectValue(value, field->type));
    widget->SetValue(value);
  }
  QVariant GetValue() const override { return ToVariant(field->GetValue()); }

private:
  InspectFieldPtr field;
  std::shared_ptr<IInspectWidget> widget;
};

std::vector<std::shared_ptr<IInspectWidget>> AdaptFields(const std::vector<InspectFieldPtr>& fields)
{
  std::vector<std::shared_ptr<IInspectWidget>> adapted;
  adapted.reserve(fields.size());
  for (const InspectFieldPtr& field : fields)
  {
    if (field) adapted.push_back(std::make_shared<QtInspectFieldAdapter>(field));
  }
  return adapted;
}
}

QTSceneInspector::QTSceneInspector(QObject *parent)
    : QObject(parent)
{
  syncTimer.setInterval(33);
  syncTimer.setTimerType(Qt::PreciseTimer);
  connect(&syncTimer, &QTimer::timeout, this, [this]()
          { SyncSnapshots(); });
  syncTimer.start();
}

std::string QTSceneInspector::selectedObjectName() const
{
  return selectedProviderName;
}

/**
 * @brief Set the selected object to be the one with the given name.
 *  If no object has the name, selection will be cleared.
 *
 * @param name
 */
void QTSceneInspector::setSelectedObjectName(const std::string &name)
{
  if (name == selectedProviderName)
  {
    return;
  }

  // Find the provider that has name matching the input name
  bool found = false;
  for (auto provider : providers)
  {
    if (provider && provider->GetInspectDisplayName() == name)
    {
      found = true;
      selectedProviderName = name;
      currentFields = AdaptFields(provider->GetInspectFields());
      break;
    }
  }

  if (!found)
  {
    // Clear selection if no provider has the input name
    selectedProviderName = "";
    currentFields.clear();
  }

  // Build the field objects for the newly selected provider
  RebuildFieldObjects();

  // Change signal emit
  emit selectedProviderIndexChanged();
  emit fieldsChanged();
  ++revision;
  emit fieldRevisionChanged();
}

QObjectList QTSceneInspector::fields() const
{
  return fieldObjects;
}

int QTSceneInspector::fieldRevision() const
{
  return revision;
}

/**
 * @brief Update the list of inspectable providers coming from the scene.
 * 
 * @param newProviders 
 */
void QTSceneInspector::Update(const std::vector<InspectProvider *> &newProviders)
{
  bool sameProviders = newProviders.size() == providers.size();
  if (sameProviders)
  {
    for (size_t i = 0; i < newProviders.size(); ++i)
    {
      if (newProviders[i] != providers[i])
      {
        sameProviders = false;
        break;
      }
    }
  }

  if (!sameProviders)
  {
    SetProviders(newProviders);
    return;
  }

  if (selectedProviderName.empty())
  {
    return;
  }

  InspectProvider *selectedProvider = FindProviderByName(selectedProviderName);
  if (!selectedProvider)
  {
    selectedProviderName.clear();
    currentFields.clear();
    RebuildFieldObjects();
    emit selectedProviderIndexChanged();
    emit fieldsChanged();
    ++revision;
    emit fieldRevisionChanged();
    return;
  }

  std::vector<std::shared_ptr<IInspectWidget>> refreshedFields = AdaptFields(selectedProvider->GetInspectFields());
  bool changed = refreshedFields.size() != currentFields.size();
  if (!changed)
  {
    for (size_t i = 0; i < refreshedFields.size(); ++i)
    {
      const std::shared_ptr<IInspectWidget> &refreshed = refreshedFields[i];
      const std::shared_ptr<IInspectWidget> &existing = currentFields[i];
      const QString refreshedId = refreshed ? refreshed->fieldId() : QString();
      const QString existingId = existing ? existing->fieldId() : QString();
      if (refreshedId != existingId)
      {
        changed = true;
        break;
      }
    }
  }

  if (!changed)
  {
    return;
  }

  currentFields = std::move(refreshedFields);
  RebuildFieldObjects();
  emit fieldsChanged();
  ++revision;
  emit fieldRevisionChanged();
}

/**
 * @brief Set the list of inspectable providers coming from the scene, replacing the existing list. 
 *  This is used when the provider list has changed (e.g. from scene updates).
 * 
 * @param newProviders 
 */
void QTSceneInspector::SetProviders(const std::vector<InspectProvider *> &newProviders)
{
  const std::string previousSelection = selectedProviderName;
  providers = newProviders;

  emit providersChanged();

  if (providers.empty())
  {
    selectedProviderName.clear();
    currentFields.clear();
  }
  else
  {
    InspectProvider *selectedProvider = nullptr;
    if (!selectedProviderName.empty())
    {
      selectedProvider = FindProviderByName(selectedProviderName);
    }

    if (!selectedProvider)
    {
      selectedProvider = providers.front();
      selectedProviderName = selectedProvider ? selectedProvider->GetInspectDisplayName() : std::string();
    }

    currentFields = selectedProvider ? AdaptFields(selectedProvider->GetInspectFields()) : std::vector<std::shared_ptr<IInspectWidget>>{};
  }

  RebuildFieldObjects();

  if (previousSelection != selectedProviderName)
  {
    emit selectedProviderIndexChanged();
  }

  emit fieldsChanged();
  ++revision;
  emit fieldRevisionChanged();
  emit visibilityStateChanged();
}

QVariantMap QTSceneInspector::fieldMeta(const QString &fieldId) const
{
  QVariantMap meta;

  const std::shared_ptr<IInspectWidget> field = FindField(fieldId);
  if (field)
  {
    meta = field->meta();
  }

  return meta;
}

QVariant QTSceneInspector::fieldValue(const QString &fieldId) const
{
  const std::shared_ptr<IInspectWidget> field = FindField(fieldId);
  if (!field)
  {
    return QVariant();
  }

  return field->GetValue();
}

bool QTSceneInspector::setFieldValue(const QString &fieldId, const QVariant &value)
{
  const std::shared_ptr<IInspectWidget> field = FindField(fieldId);
  if (!field)
  {
    return false;
  }

  field->SetValue(value);
  fieldSnapshots[field->fieldId()] = field->GetValue();
  ++revision;
  emit fieldRevisionChanged();
  return true;
}

bool QTSceneInspector::hasVisibility(const std::string &providerName) const
{
  InspectProvider *provider = FindProviderByName(providerName);
  if (!provider)
  {
    return false;
  }

  return provider->HasVisibility();
}

bool QTSceneInspector::isVisible(const std::string &providerName) const
{
  InspectProvider *provider = FindProviderByName(providerName);
  if (!provider)
  {
    return false;
  }

  return provider->IsVisible();
}

bool QTSceneInspector::setVisible(const std::string &providerName, bool visible)
{
  InspectProvider *provider = FindProviderByName(providerName);
  if (!provider)
  {
    return false;
  }

  if (!provider->HasVisibility())
  {
    return false;
  }

  const std::vector<std::shared_ptr<IInspectWidget>> providerFields = AdaptFields(provider->GetInspectFields());
  const std::shared_ptr<IInspectWidget> visibilityField = FindVisibilityField(providerFields);
  if (!visibilityField)
  {
    return false;
  }

  visibilityField->SetValue(visible);

  if (selectedProviderName == providerName)
  {
    const std::shared_ptr<IInspectWidget> selectedField = FindField(visibilityField->fieldId());
    if (selectedField)
    {
      selectedField->SetValue(visible);
      fieldSnapshots[selectedField->fieldId()] = selectedField->GetValue();
    }
  }

  ++revision;
  emit fieldRevisionChanged();
  emit visibilityStateChanged();
  return true;
}

/**
 * @brief Select an object in the scene by casting a ray from the given origin in the given direction. 
 *        The closest intersecting object will be selected. Objects are intersected based on their CastRay implementation, 
 *          which typically uses bounding volumes for hit testing.
 * 
 * @param rayOrigin 
 * @param rayDirection 
 * @return true 
 * @return false 
 */
bool QTSceneInspector::selectObjectByRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection)
{
  const float directionLength = glm::length(rayDirection);
  if (directionLength <= 1e-6f)
  {
    return false;
  }

  const glm::vec3 normalizedDirection = rayDirection / directionLength;
  float closestDistance = std::numeric_limits<float>::max();
  std::string closestProviderName;

  for (InspectProvider *provider : providers)
  {
    if (!provider)
    {
      continue;
    }

    const std::optional<float> hitDistance = provider->CastRay(rayOrigin, normalizedDirection);
    if (!hitDistance.has_value())
    {
      continue;
    }

    const float distance = hitDistance.value();
    if (distance < 0.0f)
    {
      continue;
    }

    if (distance < closestDistance)
    {
      closestDistance = distance;
      closestProviderName = provider->GetInspectDisplayName();
    }
  }

  if (closestProviderName.empty())
  {
    return false;
  }

  setSelectedObjectName(closestProviderName);
  return true;
}

void QTSceneInspector::RebuildFieldObjects()
{
  fieldObjects.clear();
  fieldSnapshots.clear();
  for (const std::shared_ptr<IInspectWidget> &field : currentFields)
  {
    if (!field)
    {
      continue;
    }

    auto *obj = dynamic_cast<QObject *>(field.get());
    if (QCoreApplication::instance() && obj && obj->thread() != QCoreApplication::instance()->thread())
    {
      obj->moveToThread(QCoreApplication::instance()->thread());
    }

    if (obj)
    {
      fieldObjects.push_back(obj);
      fieldSnapshots.insert(field->fieldId(), field->GetValue());
    }
  }
}

void QTSceneInspector::SyncSnapshots()
{
  for (const std::shared_ptr<IInspectWidget> &field : currentFields)
  {
    if (!field)
    {
      continue;
    }

    const QVariant currentValue = field->GetValue();
    const QVariant previousValue = fieldSnapshots.value(field->fieldId());
    if (currentValue != previousValue)
    {
      fieldSnapshots.insert(field->fieldId(), currentValue);
      emit fieldValueChanged(field->fieldId(), currentValue);
      ++revision;
      emit fieldRevisionChanged();
    }
  }
}

std::shared_ptr<IInspectWidget> QTSceneInspector::FindField(const QString &fieldId) const
{
  for (const std::shared_ptr<IInspectWidget> &field : currentFields)
  {
    if (field && field->fieldId() == fieldId)
    {
      return field;
    }
  }
  return nullptr;
}

InspectProvider *QTSceneInspector::FindProviderByName(const std::string &name) const
{
  for (InspectProvider *provider : providers)
  {
    if (provider && provider->GetInspectDisplayName() == name)
    {
      return provider;
    }
  }
  return nullptr;
}

std::shared_ptr<IInspectWidget> QTSceneInspector::FindVisibilityField(const std::vector<std::shared_ptr<IInspectWidget>> &fields) const
{
  for (const std::shared_ptr<IInspectWidget> &field : fields)
  {
    if (!field)
    {
      continue;
    }

    const QString id = field->fieldId();
    if (id == "visible" || id == "isVisible")
    {
      return field;
    }
  }
  return nullptr;
}
