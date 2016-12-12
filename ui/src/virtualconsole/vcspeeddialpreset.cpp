/*
  Q Light Controller Plus
  vcspeeddialpreset.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "vcspeeddialpreset.h"
#include "vcwidget.h"
#include "qlcfile.h"

VCSpeedDialPreset::VCSpeedDialPreset(quint8 id)
    : m_id(id)
    , m_value(1000)
{
}

VCSpeedDialPreset::VCSpeedDialPreset(VCSpeedDialPreset const& preset)
    : m_id(preset.m_id)
    , m_name(preset.m_name)
    , m_value(preset.m_value)
    , m_keySequence(preset.m_keySequence)
{
    if (preset.m_inputSource != NULL)
    {
        m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(preset.m_inputSource->universe(),
                                                       preset.m_inputSource->channel()));
        m_inputSource->setRange(preset.m_inputSource->lowerValue(), preset.m_inputSource->upperValue());
    }
}

VCSpeedDialPreset::~VCSpeedDialPreset()
{
}

bool VCSpeedDialPreset::operator<(VCSpeedDialPreset const& right) const
{
    return m_id < right.m_id;
}

bool VCSpeedDialPreset::compare(VCSpeedDialPreset const* left, VCSpeedDialPreset const* right)
{
    return *left < *right;
}

bool VCSpeedDialPreset::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCSpeedDialPreset)
    {
        qWarning() << Q_FUNC_INFO << "Speed Dial Preset node not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCSpeedDialPresetID) == false)
    {
        qWarning() << Q_FUNC_INFO << "Speed Dial Preset ID not found";
        return false;
    }

    m_id = root.attributes().value(KXMLQLCVCSpeedDialPresetID).toString().toUInt();

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCSpeedDialPresetName)
        {
            m_name = root.readElementText();
        }
        else if (root.name() == KXMLQLCVCSpeedDialPresetValue)
        {
            m_value = root.readElementText().toInt();
        }
        else if (root.name() == KXMLQLCVCSpeedDialPresetInput)
        {
            QXmlStreamAttributes attrs = root.attributes();

            if (attrs.hasAttribute(KXMLQLCVCSpeedDialPresetInputUniverse) &&
                attrs.hasAttribute(KXMLQLCVCSpeedDialPresetInputChannel))
            {
                quint32 uni = attrs.value(KXMLQLCVCSpeedDialPresetInputUniverse).toString().toUInt();
                quint32 ch = attrs.value(KXMLQLCVCSpeedDialPresetInputChannel).toString().toUInt();
                m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));

                uchar min = 0, max = UCHAR_MAX;
                if (attrs.hasAttribute(KXMLQLCVCWidgetInputLowerValue))
                    min = uchar(attrs.value(KXMLQLCVCWidgetInputLowerValue).toString().toUInt());
                if (attrs.hasAttribute(KXMLQLCVCWidgetInputUpperValue))
                    max = uchar(attrs.value(KXMLQLCVCWidgetInputUpperValue).toString().toUInt());
                m_inputSource->setRange(min, max);
            }
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCSpeedDialPresetKey)
        {
            m_keySequence = VCWidget::stripKeySequence(QKeySequence(root.readElementText()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCSpeedDialPreset tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCSpeedDialPreset::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCVCSpeedDialPreset);
    doc->writeAttribute(KXMLQLCVCSpeedDialPresetID, QString::number(m_id));

    doc->writeTextElement(KXMLQLCVCSpeedDialPresetName, m_name);
    doc->writeTextElement(KXMLQLCVCSpeedDialPresetValue, QString::number(m_value));

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
    {
        doc->writeStartElement(KXMLQLCVCSpeedDialPresetInput);
        doc->writeAttribute(KXMLQLCVCSpeedDialPresetInputUniverse, QString("%1").arg(m_inputSource->universe()));
        doc->writeAttribute(KXMLQLCVCSpeedDialPresetInputChannel, QString("%1").arg(m_inputSource->channel()));
        if (m_inputSource->lowerValue() != 0)
            doc->writeAttribute(KXMLQLCVCWidgetInputLowerValue, QString::number(m_inputSource->lowerValue()));
        if (m_inputSource->upperValue() != UCHAR_MAX)
            doc->writeAttribute(KXMLQLCVCWidgetInputUpperValue, QString::number(m_inputSource->upperValue()));
        doc->writeEndElement();
    }

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCSpeedDialPresetKey, m_keySequence.toString());

    /* End the <Preset> tag */
    doc->writeEndElement();

    return true;
}

