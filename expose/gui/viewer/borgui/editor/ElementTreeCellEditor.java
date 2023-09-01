package borgui.editor;

import borgui.xml.*;
import org.jdom2.Attribute;
import org.jdom2.Element;

import javax.swing.*;
import javax.swing.text.InternationalFormatter;
import javax.swing.tree.DefaultTreeCellEditor;
import javax.swing.tree.DefaultTreeCellRenderer;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.EventObject;
import java.util.Vector;


/**
 * A TreeCellEditor.
 */
public class ElementTreeCellEditor  extends DefaultTreeCellEditor
{
    protected BorealisModel m_borealisModel;

    
    /**
     * Constructor.
     */
    public ElementTreeCellEditor(JTree                    tree,
                                 DefaultTreeCellRenderer  renderer)
    {
        super(tree, renderer);
    }

    
    /**
     * Configures the editor. Passed onto the realEditor.
     */
    public Component getTreeCellEditorComponent(JTree      tree,
                                                Object     value,
                                                boolean    isSelected,
                                                boolean    expanded,
                                                boolean    leaf,
                                                int        row)
   {
        Component comp = null;
        
        if (value instanceof Attribute)
        {
            comp = new AttributeEditor((Attribute) value);
            
            if (isSelected)
            {
                comp.setBackground(Color.lightGray);
            }
            else
            {
                comp.setBackground(tree.getBackground());
            }
        }
        // elements are not editable
        else if (value instanceof Element)
        {
            comp = tree.getCellRenderer().getTreeCellRendererComponent(
                    tree, value,
                    isSelected, expanded,
                    leaf, row, true);
        }
        
        return comp;
        
    }
    

    public void setBorealisModel(BorealisModel borealisModel)
    {
        m_borealisModel = borealisModel;
    }

    protected boolean canEditImmediately(EventObject event) {
        return true;
    }
 
    /**
     * A class that represent the attribute editor.
     */
    private class AttributeEditor  extends JPanel
                                implements PropertyChangeListener
    {
        protected Attribute            m_attr;
        protected JFormattedTextField  m_field;
        
        
        /**
         * Constructor.
         */
        public AttributeEditor(Attribute attr)
        {
            m_attr = attr;
            
            setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
            
            // name is uneditable
            JLabel label = new JLabel("<html><font color=red>" +
                    attr.getName() +
                    "</font>" +
                    "=</html>");
            add(label);

            Vector<String> optionStrs = new Vector<>();
            boolean dropdown = false;
            Element parent = attr.getParent();
            if (parent instanceof ExperimentTaskElement) {
                String task = parent.getName();
                if (attr.getName().equals("node")) {
                } else if (task.equals("deployQueries")) {
                    if (attr.getName().equals("queryId")) {
                        // Select existing query from dropdown
                        dropdown = true;
                        for (QueryElement qe : NetworkElement.queryElements) {
                            optionStrs.add(qe.getId());
                        }
                    } else if (attr.getName().equals("uniqueQueryId")) {

                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("addPotentialHost")) {
                    if (attr.getName().equals("nodeId")) {

                    } else if (attr.getName().equals("uniqueQueryId")) {
                        // Select existing query from dropdown
                        dropdown = true;
                        for (Integer uniqueId : ExperimentTaskElement.deployedQueries.keySet()) {
                            if (ExperimentTaskElement.deployedQueries.get(uniqueId).getId() < ((ExperimentTaskElement) parent).getId()) {
                                optionStrs.add(uniqueId.toString());
                            }
                        }
                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("addSourceNode")) {
                    if (attr.getName().equals("nodeId")) {

                    } else if (attr.getName().equals("uniqueQueryId")) {
                        // Select existing query from dropdown
                        dropdown = true;
                        for (Integer uniqueId : ExperimentTaskElement.deployedQueries.keySet()) {
                            if (ExperimentTaskElement.deployedQueries.get(uniqueId).getId() < ((ExperimentTaskElement) parent).getId()) {
                                optionStrs.add(uniqueId.toString());
                            }
                        }
                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("addSinkNode")) {
                    if (attr.getName().equals("nodeId")) {

                    } else if (attr.getName().equals("uniqueQueryId")) {
                        // Select existing query from dropdown
                        dropdown = true;
                        for (Integer uniqueId : ExperimentTaskElement.deployedQueries.keySet()) {
                            if (ExperimentTaskElement.deployedQueries.get(uniqueId).getId() < ((ExperimentTaskElement) parent).getId()) {
                                optionStrs.add(uniqueId.toString());
                            }
                        }
                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("sendDsAsStream") || task.equals("sendDsAsSpecificStream")) {
                    if (attr.getName().equals("datasetId")) {
                        // Select existing query from dropdown
                        dropdown = true;
                        for (DatasetElement de : NetworkElement.datasetElements) {
                            optionStrs.add(de.getAttributeValue("ID"));
                        }
                    } else if (attr.getName().equals("iterations")) {

                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("sendNRowsDsAsSpecificStream")) {
                    if (attr.getName().equals("datasetId")) {
                        // Select existing query from dropdown
                        dropdown = true;
                        for (DatasetElement de : NetworkElement.datasetElements) {
                            optionStrs.add(de.getAttributeValue("ID"));
                        }
                    } else if (attr.getName().equals("iterations")) {

                    } else if (attr.getName().equals("number-rows")) {

                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("disconnectNode")) {
                    if (attr.getName().equals("msBeforeDisconnect")) {

                    } else if (attr.getName().equals("msBeforeNotifyDisconnect")) {

                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("retEndOfStream")) {
                    if (attr.getName().equals("ms")) {

                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("wait")) {
                    if (attr.getName().equals("ms")) {

                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("loopTasks")) {
                    if (attr.getName().equals("iterations")) {

                    } else {
                        System.exit(88);
                    }
                } else if (task.equals("adapt")) {
                    if (attr.getName().equals("queryId")) {
                        // Select existing query from dropdown
                        dropdown = true;
                        for (QueryElement qe : NetworkElement.queryElements) {
                            optionStrs.add(qe.getId());
                        }
                    } else if (attr.getName().equals("nodeId")) {
                    } else if (attr.getName().equals("migrationMode")) {
                    } else {
                        System.exit(88);
                    }
                }
            }

            if (dropdown) {
                JComboBox comboBox = new JComboBox(optionStrs);
                comboBox.addActionListener (e -> m_attr.setValue(comboBox.getItemAt(comboBox.getSelectedIndex()).toString()));
                add(comboBox);
            }
            else {
                // value is editable
                m_field = new JFormattedTextField(new InternationalFormatter());
                m_field.setValue(attr.getValue());
                m_field.setColumns(10);
                m_field.addPropertyChangeListener("value", this);
                add(m_field);
            }
        }
       
 
        /** Called when a field's "value" property changes. */
        public void propertyChange(PropertyChangeEvent e)
        {
            String oldId = m_attr.getValue();
            m_attr.setValue(m_field.getValue().toString());
            if (m_attr.getParent() instanceof ExperimentTaskElement) {
                if (m_attr.getParent().getName().equals("deployQueries")) {
                    if (m_attr.getName().equals("uniqueQueryId")) {
                        ExperimentTaskElement.deployedQueries.remove(oldId);
                        ExperimentTaskElement.deployedQueries.put(Integer.parseInt(m_field.getValue().toString()), (ExperimentTaskElement) m_attr.getParent());
                    }
                }
            }
            
            if(m_borealisModel!=null)
            {
                String attrName = m_attr.getName();
                Element element = ((Attribute)m_attr).getParent();
                String elementName = element.getName();
                String oldValue = (String) e.getOldValue();
                String newValue = (String) e.getNewValue();
                
                if(elementName.equals(XmlConstants.SCHEMA_ELEMENT))
                {
                    if(attrName.equals(XmlConstants.NAME_ATTRIBUTE))
                    {
                        m_borealisModel.changeSchemaElementName(oldValue, newValue);
                    }
                }
                else if(elementName.equals(XmlConstants.INPUT_ELEMENT))
                {
                    if(attrName.equals(XmlConstants.INPUT_STREAM_ATTRIBUTE))
                    {
                        m_borealisModel.changeInputElementStream(
                                   (InputElement)element, oldValue, newValue);
                    }
                    else if(attrName.equals(XmlConstants.INPUT_SCHEMA_ATTRIBUTE))
                    {
                        m_borealisModel.changeElementSchemaAttribute(newValue);
                    }
                    else if(attrName.equals(XmlConstants.INPUT_NODE_ATTRIBUTE))
                    {
                         JOptionPane.showConfirmDialog(null,
                                "Please change the node from the context menu",
                                "Error on input node", JOptionPane.CLOSED_OPTION,
                                JOptionPane.ERROR_MESSAGE);
                    }
                }
                else if(elementName.equals(XmlConstants.OUTPUT_ELEMENT))
                {
                    if(attrName.equals(XmlConstants.OUTPUT_STREAM_ATTRIBUTE))
                    {
                        m_borealisModel.changeOutputElementStream((OutputElement)element, oldValue, newValue);
                        
                    }
                    else if(attrName.equals(XmlConstants.OUTPUT_SCHEMA_ATTRIBUTE))
                    {
                        m_borealisModel.changeElementSchemaAttribute(newValue);
                    }
                }
                else if(elementName.equals(XmlConstants.IN_ELEMENT))
                {
                    m_borealisModel.changeInElementStream((BoxElement)element.getParentElement(), oldValue, newValue);
                }
                else if(elementName.equals(XmlConstants.OUT_ELEMENT))
                {
                    m_borealisModel.changeOutElementStream((BoxElement)element.getParentElement(), oldValue, newValue);
                }
                else if(elementName.equals(XmlConstants.BOX_ELEMENT))
                {
                    if(attrName.equals(XmlConstants.NAME_ATTRIBUTE))
                    {
                        m_borealisModel.changeBoxElementName((BoxElement)element, newValue);
                    }
                    else if(attrName.equals(XmlConstants.BOX_TYPE_ATTRIBUTE))
                    {
                        JOptionPane.showConfirmDialog(null,
                                "You don't want to change the box type. Please remove it and add a new box with the proper type",
                                "Error on box type", JOptionPane.CLOSED_OPTION,
                                JOptionPane.ERROR_MESSAGE);
                    }
                    else if(attrName.equals(XmlConstants.BOX_NODE_ATTRIBUTE))
                    {
                        JOptionPane.showConfirmDialog(null,
                                "Please change the node from the context menu",
                                "Error on box node", JOptionPane.CLOSED_OPTION,
                                JOptionPane.ERROR_MESSAGE);
                    }
                }
                else if(elementName.equals(XmlConstants.TABLE_ELEMENT))
                {
                    if(attrName.equals(XmlConstants.NAME_ATTRIBUTE))
                    {
                        m_borealisModel.changeTableElementName((TableElement)element, newValue);
                    }
                    else if(attrName.equals(XmlConstants.TABLE_STREAM_ATTRIBUTE))
                    {
                        m_borealisModel.changeTableElementStream((TableElement)element, newValue);
                        
                    }
                    else if(attrName.equals(XmlConstants.TABLE_SCHEMA_ATTRIBUTE))
                    {
                        m_borealisModel.changeElementSchemaAttribute(newValue);
                    }
                    else if(attrName.equals(XmlConstants.TABLE_SCHEMA_ATTRIBUTE))
                    {
                        JOptionPane.showConfirmDialog(null,
                                "Please change the node from the context menu",
                                "Error on table node", JOptionPane.CLOSED_OPTION,
                                JOptionPane.ERROR_MESSAGE);
                    }
                }
                else if(elementName.equals(XmlConstants.ACCESS_ELEMENT))
                {
                    if(attrName.equals(XmlConstants.ACCESS_TABLE_ATTRIBUTE))
                    {
                        JOptionPane.showConfirmDialog(null,
                                "Please change the table reference in the graph.",
                                "Error on table", JOptionPane.CLOSED_OPTION,
                                JOptionPane.ERROR_MESSAGE);
                        
                    }
                }
                else if(elementName.equals(XmlConstants.FIELD_ELEMENT))
                {
                    if(attrName.equals(XmlConstants.FIELD_TYPE_ATTRIBUTE))
                    {
                         JOptionPane.showConfirmDialog(null,
                                "You don't want to change the field type. Please remove it and add a field with the proper type.",
                                "Error on field", JOptionPane.CLOSED_OPTION,
                                JOptionPane.ERROR_MESSAGE);
                    }
                }
            }
        }
    }
}


/////////////////////  end  ElementTreeCellEditor.java  ////////////////////////
