package borgui.editor;

import borgui.xml.*;
import org.jdom2.Attribute;
import org.jdom2.Element;

import javax.swing.*;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;


/**
 * A class that represents the element tree editor.
 */
public class ExperimentTreeEditor extends JPanel implements TreeSelectionListener
{
    protected BorealisElement       m_element;
    protected BorealisElement       m_root;
    protected TaskFlowSetElement m_rootTaskFlowElement;
    protected Vector<ExperimentTaskElement>        m_addElements;
    protected JPanel                m_optionPanel;
    protected JTree                 m_experimentTree;
    protected ExperimentTreeModel   m_experimentTreeModel;
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
    public ExperimentTreeEditor(BorealisModel borealisModel)
    {
        super();

        m_borealisModel = borealisModel;
        m_element       = new TaskFlowSetElement();
        m_root = m_element;
        m_addElements   = new Vector<>();

        this.setup();
    }


    public ExperimentTreeEditor(BorealisModel    borealisModel,
                                ExperimentElement  element,
                                Vector<ExperimentTaskElement> addElements)
    {
        super();

        m_borealisModel = borealisModel;
        m_element       = element;
        m_rootTaskFlowElement = element.rootTaskElement;
        m_root = element;
        m_addElements   = addElements;

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
        m_experimentTreeModel = new ExperimentTreeModel(m_element);
        // tree
        m_experimentTree = new JTree(m_experimentTreeModel);
        m_experimentTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        m_experimentTree.addTreeSelectionListener(this);


        //custom renderers and editors
        DefaultTreeCellRenderer cellRenderer = new ElementTreeCellRenderer();
        m_experimentTree.setCellRenderer(cellRenderer);
        ElementTreeCellEditor elementTreeCellEditor = new ElementTreeCellEditor(m_experimentTree, cellRenderer);
        elementTreeCellEditor.setBorealisModel(m_borealisModel);
        m_experimentTree.setCellEditor(elementTreeCellEditor);
        
        
        // make editable
        m_experimentTree.setEditable(true);
        m_experimentTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        m_experimentTree.setShowsRootHandles(true);
        
        
        m_editorScrollPane = new JScrollPane(m_experimentTree);
        m_editorScrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
        m_editorScrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        
        add(m_editorScrollPane);
        
        // options
        m_optionPanel = new JPanel(new SpringLayout());
        //m_optionPanel.setPreferredSize(new Dimension(100, 100));
        m_optionPanel.setLayout(new BoxLayout(m_optionPanel, BoxLayout.X_AXIS));
        
        m_addButton = new JButton("Add");
        m_optionPanel.add(m_addButton);

        m_addElements.add(new ExperimentTaskElement("deployQueries", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("addPotentialHost", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("addSourceNode", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("addSinkNode", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("sendDsAsSpecificStream", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("sendNRowsDsAsSpecificStream", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("disconnectNode", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("retEndOfStream", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("wait", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("loopTasks", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("batchTasks", new ArrayList<>()));
        m_addElements.add(new ExperimentTaskElement("adapt", new ArrayList<>()));

        Vector<String>  optionStrs = new Vector<>();

        for(ExperimentTaskElement option : m_addElements) {
            optionStrs.add(option.getName());
        }

        m_addCombo = new JComboBox(optionStrs);
        m_optionPanel.add(m_addCombo);
        //m_nodeTextField.setMaximumSize(new Dimension(20, 20));
        m_optionPanel.add(m_nodeField);

        m_removeButton = new JButton("delete");
        m_optionPanel.add(m_removeButton);

        m_addButton.addActionListener(new ExperimentTreeEditor.AddButtonListener());
        m_removeButton.addActionListener(new ExperimentTreeEditor.RemoveButtonListener());

        m_optionPanel.setMaximumSize(new Dimension(m_optionPanel.getPreferredSize().width, 20));
        this.add(m_optionPanel);
    }


    /**
     * Set the Borealis Element of the tree.
     */
    public void setElement(BorealisElement element, Vector addElements)
    {
        m_element     = element;
        m_addElements = addElements;

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

        m_experimentTreeModel.setElement(m_element);
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

            if (p instanceof ExperimentTaskElement) {
                ExperimentTaskElement experimentTaskElement = (ExperimentTaskElement) p;
                m_element = experimentTaskElement;
                break;
            } else if (p instanceof TaskFlowSetElement) {
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
            ExperimentTaskElement element = (ExperimentTaskElement) m_addElements.get(m_addCombo.getSelectedIndex()).clone();
            ExperimentTaskElement newElement = new ExperimentTaskElement(element.getName(), new ArrayList<>());
            newElement.setNode((int)m_nodeField.getValue());
            newElement.buildNewElement();

            if (m_element instanceof ExperimentTaskElement) {
                Element p = m_element.getParentElement();
                m_experimentTreeModel.insertNodeInto(newElement, p, p.indexOf(m_element) + 1);
            } else if (m_element instanceof TaskFlowSetElement) {
                Element p = m_element;
                m_experimentTreeModel.insertNodeInto(newElement, p, p.getContentSize());
            } else {
                Element p = m_rootTaskFlowElement;
                m_experimentTreeModel.insertNodeInto(newElement, p, p.getContentSize());
            }

            m_experimentTreeModel.showTreeUpdate();
            for (int i = 0; i < m_experimentTree.getRowCount(); i++) {
                m_experimentTree.expandRow(i);
            }
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
            TreePath path = m_experimentTree.getSelectionPath();

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
                        BorealisElement elementToRemove = (BorealisElement) path.getLastPathComponent();
                        if (m_element == elementToRemove) {
                            m_element = (TaskFlowSetElement) m_element.getParent();
                        }
                        elementToRemove.getParent().removeContent(elementToRemove);
                        // Hack! new_m_element only has to be set because m_experimentTreeModel.showTreeUpdate() fires the method
                        // earlier that sets m_element. It has to do with a semantic confusion where this method that gets called
                        // thinks a new task has been clicked, but the showTreeUpdate() triggers it for the task that's being
                        // deleted. I don't know how it should be fixed, but it's not "good"
                        BorealisElement new_m_element = m_element;
                        m_experimentTreeModel.showTreeUpdate();
                        m_element = new_m_element;
                    }

                    if (!deleteResult)
                    {
                        System.out.println("Error in deleting element.");
                    }
                }
            }

            for (int i = 0; i < m_experimentTree.getRowCount(); i++) {
                m_experimentTree.expandRow(i);
            }
        }
    }


    public void setEditable(boolean bool)
    {
        m_experimentTree.setEditable(bool);
        m_addCombo.setEnabled(bool);
        m_addButton.setEnabled(bool);
        m_removeButton.setEnabled(bool);
    }


    public void showTreeUpdate()
    {
        m_experimentTreeModel.showTreeUpdate();
    }
}


////////////////////////  end ElementTreeEditor.java  //////////////////////////
