package borgui.editor;

import borgui.xml.*;
import org.jdom2.Element;

import javax.swing.*;
import java.awt.*;
import java.awt.event.KeyEvent;
import java.util.Iterator;
import java.util.Vector;

/**
 * A class that represents a element panel.
 */
public class ExperimentPanel extends JPanel
{
    protected ExperimentTreeEditor  m_experimentTreeEditor;
    protected XmlEditor          m_xmlEditor;
    protected DeployTreeEditor   m_deployXmlEditor = null;

    protected Element            m_element;
    protected JTabbedPane        m_tabbedPane;

    protected BorealisModel      m_borealisModel;
    protected boolean            m_forEditorPanel = true;


    /**
     * Constructor.
     */
    public ExperimentPanel(BorealisModel borealisModel)
    {
        super();

        m_borealisModel = borealisModel;
        m_experimentTreeEditor = new ExperimentTreeEditor(m_borealisModel);
        //m_xmlEditor = new XmlEditor();
        
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        
        m_tabbedPane = new JTabbedPane();

        m_tabbedPane.addTab("Experiment tasks", null, m_experimentTreeEditor,"Edit experiment tasks");

        m_tabbedPane.setMnemonicAt(0, KeyEvent.VK_1);

        // add scrollPane to this panel
        add(m_tabbedPane);
        
    }
    
    public void setFocusTabDeploy()
    {
        m_tabbedPane.setSelectedIndex(2);
    }
    
    /**
     * Set the borealis element for the tree.
     */
    public void setElement(BorealisElement element)
    {
        m_element = element;
        Vector<Object> childElements = new Vector<Object>();

        if (element.getName().equals(XmlConstants.BOX_ELEMENT))
        {
            childElements.add(new ParameterElement("param_name","value"));
            //m_elementTreeEditor.getAddButton().setActionCommand("AddParam");
        }
        else if (element.getName().equals(XmlConstants.SCHEMA_ELEMENT))
        {
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_BYTE_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_SHORT_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_INT_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_LONG_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_SINGLE_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_DOUBLE_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_STRING_TYPE, 20));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_TIMESTAMP_TYPE));
            //m_elementTreeEditor.getAddButton().setActionCommand("AddField");
        }
        else if (element.getName().equals(XmlConstants.OUTPUT_ELEMENT))
        {
            SchemaElement schemaElement;
            java.util.List schemaList = m_borealisModel.getDocument().getAllSchemas();

            if (schemaList.size()!=0)
            {
                schemaElement = (SchemaElement)schemaList.get(0);
            }
            else
            {
                schemaElement = new SchemaElement();
                m_borealisModel.getDocument().addSchema(schemaElement);
            }
            
            childElements.add(schemaElement.getNameValue());
        }

        m_experimentTreeEditor.setElement(element, childElements);
        m_xmlEditor.setText(XmlHandler.elementToXml(element));
        m_tabbedPane.setSelectedIndex(0);
    }
    
    public void setEditable(boolean bool)
    {
        m_experimentTreeEditor.setEditable(bool);
    }
    
    public DeployTreeEditor getDeployTreeEditor()
    {
        return m_deployXmlEditor;
    }
    
    public void showTreeUpdate()
    {
        m_experimentTreeEditor.showTreeUpdate();
        m_deployXmlEditor.showTreeUpdate();
    }
}


////////////////////////  end ElementPanel.java  ///////////////////////////////
