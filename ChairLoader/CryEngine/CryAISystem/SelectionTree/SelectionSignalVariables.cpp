// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "SelectionSignalVariables.h"

#include <CryCore/CryCrc32.h>

bool SelectionSignalVariables::LoadFromXML(const BlockyXmlBlocks::Ptr& blocks,
                                           const SelectionVariableDeclarations& variableDecls, const XmlNodeRef& rootNode,
                                           const char* scopeName, const char* fileName)
{
	BlockyXmlNodeRef blockyNode(blocks, scopeName, rootNode, fileName);
	while (XmlNodeRef childNode = blockyNode.next())
	{
		if (!stricmp(childNode->getTag(), "Signal"))
		{
			const char* signalName = 0;
			if (childNode->haveAttr("name"))
				childNode->getAttr("name", &signalName);
			else
			{
				AIWarning("Missing 'name' attribute for tag '%s' in file '%s' at line %d.",
				          childNode->getTag(), fileName, childNode->getLine());

				return false;
			}

			const char* variableName = 0;
			if (childNode->haveAttr("variable"))
				childNode->getAttr("variable", &variableName);
			else
			{
				AIWarning("Missing 'variable' attribute for tag '%s' in file '%s' at line %d.",
				          childNode->getTag(), fileName, childNode->getLine());

				return false;
			}

			const char* value = 0;
			if (childNode->haveAttr("value"))
				childNode->getAttr("value", &value);
			else
			{
				AIWarning("Missing 'value' attribute for tag '%s' in file '%s' at line %d.",
				          childNode->getTag(), fileName, childNode->getLine());

				return false;
			}

			SelectionVariableID variableID = variableDecls.GetVariableID(variableName);
			if (variableDecls.IsDeclared(variableID))
			{
				SignalVariables::iterator it = m_signalVariables.insert(
				  SignalVariables::value_type(CCrc32::Compute(signalName), SignalVariable()));

				SignalVariable& signalVariable = it->second;

				signalVariable.valueExpr = SelectionCondition(value, variableDecls);
				signalVariable.variableID = variableID;

				if (!signalVariable.valueExpr.Valid())
				{
					AIWarning("Failed to compile expression '%s' in file '%s' at line %d.",
					          value, fileName, childNode->getLine());

					return false;
				}
			}
			else
			{
				AIWarning("Unknown variable '%s' used for signal variable in file '%s' at line '%d'.",
				          variableName, fileName, childNode->getLine());

				return false;
			}
		}
		else
		{
			AIWarning("Unexpected tag '%s' in file '%s' at line %d. 'Signal' expected.",
			          childNode->getTag(), fileName, childNode->getLine());

			return false;
		}
	}

	return true;
}

bool SelectionSignalVariables::ProcessSignal(const char* signalName, uint32 signalCRC, SelectionVariables& variables) const
{
	SignalVariables::const_iterator it = m_signalVariables.find(signalCRC);
	if (it == m_signalVariables.end())
		return false;

	while ((it != m_signalVariables.end()) && (it->first == signalCRC))
	{
		const SignalVariable& signalVariable = it->second;
		variables.SetVariable(signalVariable.variableID, signalVariable.valueExpr.Evaluate(variables));

		++it;
	}

	return true;
}
