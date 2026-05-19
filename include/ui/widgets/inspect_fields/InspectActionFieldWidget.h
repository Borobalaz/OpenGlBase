#pragma once

#include <functional>

#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QVariant>

#include "ui/widgets/inspect_fields/IInspectWidget.h"

class InspectActionFieldWidget : public QPushButton, public IInspectWidget
{
public:
	explicit InspectActionFieldWidget(QWidget *parent = nullptr)
		: QPushButton(parent)
	{
		setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
		QObject::connect(this, &QPushButton::clicked, this, [this]()
		{
			if (actionCallback)
			{
				actionCallback();
			}
		});
	}

	InspectActionFieldWidget(QString fieldId,
													 QString displayName,
													 QString groupName,
													 QWidget *parent = nullptr)
		: InspectActionFieldWidget(parent)
	{
		fieldIdValue = std::move(fieldId);
		displayNameValue = std::move(displayName);
		groupNameValue = std::move(groupName);
		setText(displayNameValue);
	}

	IInspectWidget *addToLayout(QHBoxLayout *layout) override
	{
		layout->addWidget(this, 1);
		return this;
	}

	QString fieldId() const override { return fieldIdValue; }
	QString displayName() const override { return displayNameValue; }
	QString groupName() const override { return groupNameValue; }
	bool isReadOnly() const override { return true; }

	void SetValue(const QVariant &) override
	{
	}

	QVariant GetValue() const override
	{
		return QVariant();
	}

	std::function<void()> actionCallback;

private:
	QString fieldIdValue;
	QString displayNameValue;
	QString groupNameValue;
};
