package borgui.editor;

import borgui.xml.*;

import javax.swing.*;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Vector;


/**
 * A class that represents the element tree editor.
 */
public class NetworkTreeEditor extends JPanel implements TreeSelectionListener
{
    protected BorealisElement       m_element;
    protected BorealisElement       m_root;
    protected JPanel                m_optionPanel;
    protected JTree                 m_networkTree;
    protected NetworkTreeModel      m_networkTreeModel;
    protected JComboBox             m_addCombo;
    SpinnerModel model = new SpinnerNumberModel(0, 0, 999, 1);
    JSpinner m_nodeField = new JSpinner(model);
    protected JButton               m_addButton;
    protected JButton               m_removeButton;
    protected JScrollPane           m_editorScrollPane;
    protected BorealisModel         m_borealisModel;


    /**
     * Default constructor.
     */
    public NetworkTreeEditor(BorealisModel borealisModel)
    {
        super();

        m_borealisModel = borealisModel;
        m_element       = new TaskFlowSetElement();
        m_root = m_element;

        this.setup();
    }


    public NetworkTreeEditor(BorealisModel    borealisModel,
                             BorealisElement  element)
    {
        super();

        m_borealisModel = borealisModel;
        m_element       = element;
        m_root = m_element;

        this.setup();
    }

    /**
     * Used by constructor to set up the layout.
     */
    private void setup()
    {
        this.removeAll();
        this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

        // tree model
        m_networkTreeModel = new NetworkTreeModel(m_borealisModel.m_doc);
        // tree
        m_networkTree = new JTree(m_networkTreeModel);
        m_networkTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        m_networkTree.addTreeSelectionListener(this);


        //custom renderers and editors
        DefaultTreeCellRenderer cellRenderer = new ElementTreeCellRenderer();
        m_networkTree.setCellRenderer(cellRenderer);
        //NetworkTreeCellRenderer networkTreeCellEditor = new NetworkTreeEditor(m_borealisModel, m_element);
        //m_networkTree.setCellEditor(networkTreeCellEditor);
        
        
        // make editable
        m_networkTree.setEditable(true);
        m_networkTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        m_networkTree.setShowsRootHandles(true);
        
        
        m_editorScrollPane = new JScrollPane(m_networkTree);
        m_editorScrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
        m_editorScrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        
        add(m_editorScrollPane);
        
        // options
        m_optionPanel = new JPanel();
        m_optionPanel.setLayout(new BoxLayout(m_optionPanel, BoxLayout.X_AXIS));

        //m_addButton.addActionListener(new NetworkTreeEditor.AddButtonListener());
        //m_removeButton.addActionListener(new NetworkTreeEditor.RemoveButtonListener());

        this.add(m_optionPanel);
    }


    /**
     * Set the Borealis Element of the tree.
     */
    public void setElement(BorealisElement element, Vector addElements)
    {
        m_element     = element;

        m_addCombo.removeAllItems();

        if (element.getName().equals(XmlConstants.BOX_ELEMENT))
        {
            m_addCombo.addItem(XmlConstants.PARAMETER_ELEMENT);
            m_addCombo.setSelectedIndex(0);
            m_addButton.setActionCommand("AddParam");

        }
        else if(element.getName().equals(XmlConstants.SCHEMA_ELEMENT))
        {
            m_addCombo.addItem("field(" + XmlConstants.SCHEMA_BYTE_TYPE + ")");
            m_addCombo.addItem(XmlConstants.SCHEMA_SHORT_TYPE);
            m_addCombo.addItem(XmlConstants.SCHEMA_INT_TYPE);
            m_addCombo.addItem(XmlConstants.SCHEMA_LONG_TYPE);
            m_addCombo.addItem(XmlConstants.SCHEMA_SINGLE_TYPE);
            m_addCombo.addItem(XmlConstants.SCHEMA_DOUBLE_TYPE );
            m_addCombo.addItem(XmlConstants.SCHEMA_STRING_TYPE );
            m_addCombo.addItem(XmlConstants.SCHEMA_TIMESTAMP_TYPE );
            m_addCombo.setSelectedIndex(0);

            m_addButton.setActionCommand("AddField");
        }
        else if(element.getName().equals(XmlConstants.OUTPUT_ELEMENT))
        {
            m_addCombo.addItem(XmlConstants.SCHEMA_ELEMENT);
            m_addCombo.setSelectedIndex(0);
            m_addButton.setActionCommand("AddSchemaToOutput");
        }

        //m_networkTreeModel.setElement(m_element);
    }


    /**
     * Return the Borealis Element in the current tree.
     */
    public BorealisElement getElement()
    {
        return m_element;
    }

    
    public JComboBox getComboBox()
    {
        return m_addCombo;
    }


    public JButton getRemoveButton()
    {
        return m_removeButton;
    }


    public JButton getAddButton()
    {
        return m_addButton;
    }


    public JPanel getOptionPanel()
    {
        return m_optionPanel;
    }

    static void reverseArray(Object arr[], int size) {
        for (int i = arr.length-1; i >= arr.length/2; i--) {
            Object tmp = arr[i];
            arr[i] = arr[arr.length-1-i];
            arr[arr.length-1-i] = tmp;
        }
    }

    @Override
    public void valueChanged(TreeSelectionEvent e) {
        System.out.println("Selected " + e.getPath());
        Object[] path = e.getPath().getPath();
        reverseArray(path, path.length);
        for (Object p : path) {
            if (p instanceof TaskFlowSetElement) {
                TaskFlowSetElement taskFlowSetElement = (TaskFlowSetElement) p;
                m_element = taskFlowSetElement;
                break;
            } else if (p instanceof BorealisElement) {
                BorealisElement element = (BorealisElement) p;
                if (element.getName().equals("borealis")) {
                    m_element = element;
                    break;
                }
            }
        }
        System.out.println();
    }


    /**
     * A class that response to add action.
     */
    private class AddButtonListener implements ActionListener
    {
        //JComboBox m_combo;

        /**
         * Constructor.
         */
        public AddButtonListener() {}


        /**
         * Invoked when an action occurs.
         */
        public void actionPerformed(ActionEvent e) {
            /*ExperimentTaskElement element = (ExperimentTaskElement) m_addElements.get(m_addCombo.getSelectedIndex()).clone();
            ExperimentTaskElement newElement = new ExperimentTaskElement(element.getName(), new ArrayList<>());
            newElement.setNode((int)m_nodeField.getValue());
            newElement.buildNewElement();
            m_experimentTreeModel.insertNodeInto(newElement, m_element, m_element.getContentSize());
            for (int i = 0; i < m_experimentTree.getRowCount(); i++) {
                m_experimentTree.expandRow(i);
            }*/
        }
    }


    /**
     * A class that response to remove action.
     */
    private class RemoveButtonListener implements ActionListener
    {
        /**
         * Constructor.
         */
        public RemoveButtonListener() {}


        /**
         * Invoked when an action occurs.
         */
        public void actionPerformed(ActionEvent e)
        {
            // path to first selected node
            TreePath path = m_networkTree.getSelectionPath();

            if (path != null)
            {
                Object last = path.getLastPathComponent();

                if (last instanceof BorealisElement)
                {
                    // remove last node
                    BorealisElement deleteElement =(BorealisElement)  path.getLastPathComponent();
                    boolean deleteResult = true;

                    if (deleteElement instanceof BoxElement)
                    {
                        deleteResult = m_borealisModel.removeBoxElement(
                                (BoxElement)deleteElement);
                    }
                    else if(deleteElement instanceof TableElement)
                    {
                        deleteResult = m_borealisModel.removeTableElement(
                                (TableElement)deleteElement);
                    }
                    else if (deleteElement instanceof QueryElement)
                    {
                        deleteResult = m_borealisModel.removeQueryElement(
                                (QueryElement)deleteElement);
                    }
                    else if (deleteElement instanceof SchemaElement)
                    {
                        deleteResult = m_borealisModel.removeSchemaElement(
                                (SchemaElement)deleteElement);
                    }
                    else if (deleteElement instanceof InputElement)
                    {
                        deleteResult = m_borealisModel.removeInputElement(
                                (InputElement)deleteElement);
                    }
                    else if (deleteElement instanceof OutputElement)
                    {
                        deleteResult = m_borealisModel.removeOutputElement(
                                (OutputElement)deleteElement);
                    }
                    else if(deleteElement instanceof TablerefElement)
                    {
                        deleteResult = m_borealisModel.removeTablerefElement(
                                (TablerefElement)deleteElement);
                    }
                    else if(deleteElement instanceof InElement)
                    {
                        deleteResult = m_borealisModel.removeInElement(
                                (InElement)deleteElement);
                    }
                    else if(deleteElement instanceof OutElement)
                    {
                        deleteResult = m_borealisModel.removeOutElement(
                                (OutElement)deleteElement);
                    }
                    else
                    {
                        m_element.removeContent(
                                (BorealisElement)path.getLastPathComponent());
                    }

                    if (!deleteResult)
                    {
                        System.out.println("Error in deleting element.");
                    }

                    m_networkTreeModel.showTreeUpdate();
                }
            }

            for (int i = 0; i < m_networkTree.getRowCount(); i++) {
                m_networkTree.expandRow(i);
            }
        }
    }


    public void setEditable(boolean bool)
    {
        m_networkTree.setEditable(bool);
        m_addCombo.setEnabled(bool);
        m_addButton.setEnabled(bool);
        m_removeButton.setEnabled(bool);
    }


    public void showTreeUpdate()
    {
        m_networkTreeModel.showTreeUpdate();
    }
}


////////////////////////  end ElementTreeEditor.java  //////////////////////////
