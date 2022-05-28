// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "SelectionTreeTemplate.h"

bool SelectionTreeTemplate::LoadFromXML(const SelectionTreeTemplateID& templateID, const BlockyXmlBlocks::Ptr& blocks,
                                        const XmlNodeRef& rootNode, const char* fileName)
{
	const char* treeName;
	rootNode->getAttr("name", &treeName);
	m_name = treeName;

	BlockyXmlNodeRef blockyNode(blocks, treeName, rootNode, fileName);

	std::vector<XmlNodeRef> pending;

	while (XmlNodeRef childNode = blockyNode.next())
	{
		const char* nodeType = childNode->getTag();

		if (!stricmp(nodeType, "Blocks"))
			continue;
		else if (!stricmp(nodeType, "Variables"))
		{
			if (!m_variableDecls.LoadFromXML(blocks, childNode, treeName, fileName))
				return false;
		}
		else if (!stricmp(nodeType, "LeafTranslations"))
		{
			pending.push_back(childNode);
		}
		else if (!stricmp(nodeType, "SignalVariables"))
		{
			pending.push_back(childNode);
		}
		else if (m_selectionTree.Empty())
		{
			SelectionTree(templateID).Swap(m_selectionTree);
			m_selectionTree.ReserveNodes(1024);

			SelectionNodeID rootID = m_selectionTree.AddNode(SelectionTreeNode());
			SelectionTreeNode& root = m_selectionTree.GetNode(rootID);

			if (!root.LoadFromXML(blocks, m_selectionTree, treeName, m_variableDecls, childNode, fileName))
			{
				m_selectionTree.Clear();
				return false;
			}
		}
		else
		{
			AIWarning("Unexpected tag '%s' in file '%s' at line %d.", childNode->getTag(), fileName, childNode->getLine());

			return false;
		}
	}

	m_selectionTree.Validate();

	if (!m_selectionTree.Empty())
	{
		uint32 pendingCount = pending.size();
		for (uint32 i = 0; i < pendingCount; ++i)
		{
			XmlNodeRef& pendingNode = pending[i];
			const char* nodeType = pendingNode->getTag();

			if (!stricmp(nodeType, "LeafTranslations"))
			{
				if (!m_translator.LoadFromXML(blocks, m_selectionTree, pendingNode, treeName, fileName))
					return false;
			}
			else if (!stricmp(nodeType, "SignalVariables"))
			{
				if (!m_signalVariables.LoadFromXML(blocks, m_variableDecls, pendingNode, treeName, fileName))
					return false;
			}
		}
	}

	return true;
}

const SelectionTree& SelectionTreeTemplate::GetSelectionTree() const
{
	return m_selectionTree;
}

const SelectionVariableDeclarations& SelectionTreeTemplate::GetVariableDeclarations() const
{
	return m_variableDecls;
}

const SelectionTranslator& SelectionTreeTemplate::GetTranslator() const
{
	return m_translator;
}

const SelectionSignalVariables& SelectionTreeTemplate::GetSignalVariables() const
{
	return m_signalVariables;
}

const char* SelectionTreeTemplate::GetName() const
{
	return m_name.c_str();
}

bool SelectionTreeTemplate::Valid() const
{
	return !m_selectionTree.Empty();
}
