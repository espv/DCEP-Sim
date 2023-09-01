package borgui.editor;

import borgui.graph.BoxGraphCell;
import javax.swing.*;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import org.jdom2.Element;
import borgui.xml.*;


/**
 * A class that represents the element tree editor.
 */
public class ElementTreeEditor extends JPanel implements TreeSelectionListener
{
    protected BorealisElement   m_element;
    protected Vector            m_addElements;
    protected JPanel            m_optionPanel;
    protected JTree             m_elementTree;
    protected ElementTreeModel  m_elementTreeModel;
    protected JComboBox         m_addCombo;
    protected JSpinner          m_nodeField;
    protected JButton           m_addButton;
    protected JButton           m_removeButton;
    protected JScrollPane       m_editorScrollPane;
    protected BorealisModel     m_borealisModel;


    /**
     * Default constructor.
     */
    public ElementTreeEditor(BorealisModel borealisModel)
    {
        super();

        m_borealisModel = borealisModel;
        m_element       = new NetworkElement();
        m_addElements   = new Vector();

        this.setup();
    }


    public ElementTreeEditor(BorealisModel    borealisModel,
                             BorealisElement  element,
                             Vector addElements)
    {
        super();

        m_borealisModel = borealisModel;
        m_element       = element;
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
        m_elementTreeModel = new ElementTreeModel(m_element);
        // tree
        m_elementTree = new JTree(m_elementTreeModel);
        
        
        //custom renderers and editors
        DefaultTreeCellRenderer cellRenderer = new ElementTreeCellRenderer();
        m_elementTree.setCellRenderer(cellRenderer);
        ElementTreeCellEditor elementTreeCellEditor = new ElementTreeCellEditor(m_elementTree, cellRenderer);
        elementTreeCellEditor.setBorealisModel(m_borealisModel);
        m_elementTree.setCellEditor(elementTreeCellEditor);
        m_elementTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        m_elementTree.addTreeSelectionListener(this);
        
        
        // make editable
        m_elementTree.setEditable(true);
        m_elementTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        m_elementTree.setShowsRootHandles(true);
        
        
        m_editorScrollPane = new JScrollPane(m_elementTree);
        m_editorScrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
        m_editorScrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        
        add(m_editorScrollPane);
        
        // options
        m_optionPanel = new JPanel();
        m_optionPanel.setLayout(new BoxLayout(m_optionPanel, BoxLayout.X_AXIS));
        
        m_addButton = new JButton("Add");
        m_optionPanel.add(m_addButton);
        
        Vector<String>  optionStrs = new Vector<String>();

        for(Iterator i = m_addElements.iterator(); i.hasNext(); ) {
            optionStrs.add( ((Element) i.next()).getName() );
        }
        
        m_addCombo = new JComboBox(optionStrs);
        m_addCombo.setMaximumSize(new Dimension(1000, 25));
        m_optionPanel.add(m_addCombo);

        SpinnerModel model = new SpinnerNumberModel(0, 0, 999, 1);
        m_nodeField = new JSpinner(model);
        m_nodeField.setMaximumSize(new Dimension(25, 25));
        m_optionPanel.add(m_nodeField);
        
        m_removeButton = new JButton("delete");
        m_optionPanel.add(m_removeButton);
        
        m_addButton.addActionListener(new AddButtonListener());
        m_removeButton.addActionListener(new RemoveButtonListener());
        
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
        else if(element.getName().equals(XmlConstants.EXPERIMENT_ELEMENT))
        {
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

            for(Object option : m_addElements) {
                ExperimentTaskElement ete = (ExperimentTaskElement) option;
                m_addCombo.addItem(ete.getName());
            }
            m_addCombo.setSelectedIndex(0);

            m_addButton.setActionCommand("AddExperimentTask");
        }
        else if (element.getName().equals(XmlConstants.PLOT_ELEMENT)) {

            m_addButton.setActionCommand("AddPlot");
        }
        else if(element.getName().equals(XmlConstants.OUTPUT_ELEMENT))
        {
            m_addCombo.addItem(XmlConstants.SCHEMA_ELEMENT);
            m_addCombo.setSelectedIndex(0);
            m_addButton.setActionCommand("AddSchemaToOutput");
        }

        m_elementTreeModel.setElement(m_element);
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
        public void actionPerformed(ActionEvent e)
        {
            if (e.getActionCommand().equals("AddQuery"))
            {
                QueryElement element = (QueryElement)m_addElements.get(0);

                if (m_borealisModel.getDocument().getAllQueryNames().contains(
                                                      element.getNameValue()))
                {
                    JOptionPane.showConfirmDialog(null,
                            "The query name has already been used. Please change the name.",
                            "Change Query Name", JOptionPane.CLOSED_OPTION,
                            JOptionPane.ERROR_MESSAGE);
                }
                else
                {   m_borealisModel.getDocument().addQuery(
                          (QueryElement)(Element)element.clone());

                    JOptionPane.showConfirmDialog(null,
                            "A query has been added.",
                            "AddQuery", JOptionPane.CLOSED_OPTION,
                            JOptionPane.INFORMATION_MESSAGE);
                }
            }
            else if (e.getActionCommand().equals("AddExperimentTask")) {
                System.out.println("AddExperimentTask");
                ExperimentTaskElement element = (ExperimentTaskElement) ((ExperimentTaskElement) m_addElements.get(m_addCombo.getSelectedIndex())).clone();
                ExperimentTaskElement newElement = new ExperimentTaskElement(element.getName(), new ArrayList<>());
                newElement.setNode((int)m_nodeField.getValue());
                newElement.buildNewElement();

                if (m_element instanceof ExperimentTaskElement) {
                    Element p = m_element.getParentElement();
                    m_elementTreeModel.insertNodeInto(newElement, p, p.indexOf(m_element) + 1);
                } else if (m_element instanceof TaskFlowSetElement) {
                    Element p = m_element;
                    m_elementTreeModel.insertNodeInto(newElement, p, p.getContentSize());
                } else {
                    m_element = null;
                    for (Element c : m_elementTreeModel.m_root.getChildren()) {
                        if (c instanceof TaskFlowSetElement) {
                            m_element = (TaskFlowSetElement) c;
                            break;
                        }
                    }
                    if (m_element == null) {
                        System.out.println("Failed to find experiment flow");
                        System.exit(3);
                    }
                    m_elementTreeModel.insertNodeInto(newElement, m_element, m_element.getContentSize());
                }

                m_elementTreeModel.showTreeUpdate();
                for (int i = 0; i < m_elementTree.getRowCount(); i++) {
                    m_elementTree.expandRow(i);
                }
            }
            else if (e.getActionCommand().equals("AddPlot")) {
                System.out.println("AddPlot");
            }
            else if (e.getActionCommand().equals("AddOperator"))
            {
                // Doing everything elsewhere
            } else if (e.getActionCommand().equals("AddSchemaToOutput"))
            {
                if (((OutputElement)m_element).hasSchema())
                {
                    JOptionPane.showConfirmDialog(null,
                            "The output cell can only has one schema.",
                            "Error on output", JOptionPane.CLOSED_OPTION,
                            JOptionPane.ERROR_MESSAGE);
                }
                else
                {
                    try
                    {
                        if (m_addCombo.getItemCount()!=0)
                        {
                            String schemaName = (String) m_addElements.get(m_addCombo.getSelectedIndex());
                            ((OutputElement)m_element).setSchema(schemaName);
                        }
                    }
                    catch (Exception ex)
                    {
                        System.out.println("Can't add a element to the XML Tree");
                    }
                }
            }
            else
            {
                try
                {
                    if (m_addCombo.getItemCount()!=0)
                    {
                        Element element = (Element) m_addElements.get(m_addCombo.getSelectedIndex());
                        m_element.addContent((Element)element.clone());
                    }
                }
                catch (Exception ex)
                {
                    System.out.println("Can't add a element to the XML Tree");
                }
            }

            /*
            if (e.getActionCommand().equals("AddField"))
            {
                Element element = (Element) m_addElements.get(m_addCombo.getSelectedIndex());
                m_element.addContent((Element)element.clone());
            }
            else if(e.getActionCommand().equals("AddParam"))
            {
                Element element = (Element) m_addElements.get(m_addCombo.getSelectedIndex());
                m_element.addContent((Element)element.clone());
            }
            */

            m_elementTreeModel.showTreeUpdate();
            for (int i = 0; i < m_elementTree.getRowCount(); i++) {
                m_elementTree.expandRow(i);
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
            TreePath path = m_elementTree.getSelectionPath();

            if (path != null)
            {
                Object last = path.getLastPathComponent();

                if (last instanceof BorealisElement)
                {
                    // remove last node
                    BorealisElement deleteElement =(BorealisElement)  path.getLastPathComponent();
                    boolean deleteResult = true;

                    if (deleteElement.getParent() instanceof TaskFlowSetElement)
                    {
                        BorealisElement elementToRemove = (BorealisElement) path.getLastPathComponent();
                        if (!(elementToRemove instanceof TaskFlowSetElement)) {
                            m_element = (TaskFlowSetElement) m_element.getParent();
                        }
                        elementToRemove.getParent().removeContent(elementToRemove);
                        // Hack! new_m_element only has to be set because m_experimentTreeModel.showTreeUpdate() fires the method
                        // earlier that sets m_element. It has to do with a semantic confusion where this method that gets called
                        // thinks a new task has been clicked, but the showTreeUpdate() triggers it for the task that's being
                        // deleted. I don't know how it should be fixed, but it's not "good"
                        BorealisElement new_m_element = m_element;
                        m_elementTreeModel.showTreeUpdate();
                        m_element = new_m_element;
                    } else {
                        if (deleteElement instanceof ExperimentElement) {
                            deleteResult = m_borealisModel.removeExperimentElement((ExperimentElement) deleteElement);
                        } else if (deleteElement.getParent() instanceof ExperimentElement) {
                            deleteResult = m_borealisModel.removeExperimentElement((ExperimentElement) deleteElement.getParent());
                        } else if (deleteElement instanceof BoxElement) {
                            deleteResult = m_borealisModel.removeBoxElement(
                                    (BoxElement) deleteElement);
                        } else if (deleteElement instanceof TableElement) {
                            deleteResult = m_borealisModel.removeTableElement(
                                    (TableElement) deleteElement);
                        } else if (deleteElement instanceof QueryElement) {
                            deleteResult = m_borealisModel.removeQueryElement(
                                    (QueryElement) deleteElement);
                        } else if (deleteElement instanceof SchemaElement) {
                            deleteResult = m_borealisModel.removeSchemaElement(
                                    (SchemaElement) deleteElement);
                        } else if (deleteElement instanceof InputElement) {
                            deleteResult = m_borealisModel.removeInputElement(
                                    (InputElement) deleteElement);
                        } else if (deleteElement instanceof OutputElement) {
                            deleteResult = m_borealisModel.removeOutputElement(
                                    (OutputElement) deleteElement);
                        } else if (deleteElement instanceof TablerefElement) {
                            deleteResult = m_borealisModel.removeTablerefElement(
                                    (TablerefElement) deleteElement);
                        } else if (deleteElement instanceof InElement) {
                            deleteResult = m_borealisModel.removeInElement(
                                    (InElement) deleteElement);
                        } else if (deleteElement instanceof OutElement) {
                            deleteResult = m_borealisModel.removeOutElement(
                                    (OutElement) deleteElement);
                        } else {
                            m_element.removeContent(
                                    (BorealisElement) path.getLastPathComponent());
                        }

                        m_borealisModel.m_editorPanel.getNetworkTreeModel().showTreeUpdate();
                        m_borealisModel.m_editorPanel.setSelectedElement((BorealisElement) m_borealisModel.m_editorPanel.getNetworkTreeModel().getRoot());
                        m_borealisModel.m_editorPanel.m_elementPanel.m_elementTreeEditor.showTreeUpdate();
                        m_borealisModel.m_editorPanel.m_elementPanel.showTreeUpdate();
                    }

                    if (!deleteResult)
                    {
                        System.out.println("Error in deleting element.");
                    }

                    m_elementTreeModel.showTreeUpdate();
                }
            }

            for (int i = 0; i < m_elementTree.getRowCount(); i++) {
                m_elementTree.expandRow(i);
            }
        }
    }


    public void setEditable(boolean bool)
    {
        m_elementTree.setEditable(bool);
        m_addCombo.setEnabled(bool);
        m_addButton.setEnabled(bool);
        m_removeButton.setEnabled(bool);
    }


    public void showTreeUpdate()
    {
        m_elementTreeModel.showTreeUpdate();
    }
}


////////////////////////  end ElementTreeEditor.java  //////////////////////////
