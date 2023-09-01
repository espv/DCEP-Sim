package borgui.editor;

import borgui.common.ImageUtility;
import borgui.graph.BorealisGraphCell;
import borgui.xml.AttributeElement;
import borgui.xml.BorealisElement;
import borgui.xml.OperatorElement;
import borgui.xml.ParameterElement;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.image.BufferedImage;
import java.util.Vector;

/**
 * A class that represents a container for all the graph cell.
 */
public class GraphButtonPanel extends JPanel {
    public static final int COLS = 4;
    private GraphButtonPanel _this;
    
    public ButtonGroup m_buttonGroup;
    public SelectAction m_selectAction;
    int m_selected = 0;
    // cell count that gets put in cell label
    int[] cellCount = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EditorPanel m_editorPanel;
    public String[] icons;
    /**
     * Constructor.
     */
    public GraphButtonPanel(EditorPanel editorPanel) {
        _this = this;
        m_editorPanel = editorPanel;
        
        m_buttonGroup = new ButtonGroup();
        m_selectAction = new SelectAction();
        
        this.icons = new String[20];
        
        icons[0] = EditorConstants.INPUT_ICON;
        icons[1] = EditorConstants.OUTPUT_ICON;
        icons[2] = EditorConstants.TABLE_ICON;
        icons[3] = EditorConstants.BOX_FILTER_ICON;
        icons[4] = EditorConstants.BOX_GROUPBY_ICON;
        icons[5] = EditorConstants.BOX_CEP_ICON;
        icons[6] = EditorConstants.BOX_MAP_ICON;
        //icons[5] = EditorConstants.BOX_AGGREGATE_ICON;
        icons[7] = EditorConstants.BOX_JOIN_ICON;
        icons[8] = EditorConstants.BOX_UNION_ICON;
        icons[9] = EditorConstants.BOX_SELECT_ICON;
        icons[10] = EditorConstants.BOX_INSERT_ICON;
        icons[11] = EditorConstants.BOX_UPDATE_ICON;
        icons[12] = EditorConstants.BOX_WAITFOR_ICON;
        icons[13] = EditorConstants.BOX_RANDOMDROP_ICON;
        icons[14] = EditorConstants.BOX_WINDOWDROP_ICON;
        icons[15] = EditorConstants.BOX_USERDEFINED_ICON;
        icons[16] = EditorConstants.BOX_SPLIT_ICON;
        icons[17] = EditorConstants.BOX_WINDOW_ICON;
        icons[18] = EditorConstants.BOX_PRINT_ICON;
        icons[19] = EditorConstants.BOX_SINK_ICON;

        setBackground(EditorConstants.BACKGROUND_COLOR);
        GridLayout gridLayout = new GridLayout((int) Math.ceil((double)EditorConstants.BUTTON_NUMBER/(double) COLS), COLS);
        setLayout(gridLayout);
        
        for(int i=0; i < icons.length; i++) {
            ImageIcon icon = ImageUtility.LoadOperatorImage(icons[i]);
            JRadioButton but = new JRadioButton(icon);
            //but.setSelectedIcon(new ImageIcon(icons[i]));
            but.setActionCommand(Integer.toString(i));
            but.addActionListener(m_selectAction);
            but.setBackground(EditorConstants.BACKGROUND_COLOR);
            m_buttonGroup.add(but);
            add(but);
        }
    }
    
    /**
     * A private class that implement select action.
     */
    private class SelectAction extends AbstractAction {
        protected JRadioButton m_prevBut = null;
        protected final Color COLOR = new Color(70, 70, 200);
        
        /**
         * Constructor.
         */
        public SelectAction() {
        }
        
        /**
         * Invoked when an action occurs.
         */
        public void actionPerformed(ActionEvent e) {
            try {
                m_selected = Integer.parseInt(e.getActionCommand());
            } catch(Exception exp) {
            }
            
            if (m_prevBut != null) {
                m_prevBut.setBackground(EditorConstants.BACKGROUND_COLOR);
            }
            
            JRadioButton but = (JRadioButton) e.getSource();
            but.setBackground(COLOR);
            m_prevBut = but;
            
            BorealisGraphCell cell;
            OperatorElement elem;
            BorealisGraphCellFactory fact = new BorealisGraphCellFactory();

            switch (m_selected) {
                case EditorConstants.INPUT:
                    elem = new OperatorElement("input "+ cellCount[m_selected]++, "input");
                    elem.addContent(new ParameterElement("Schema", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.OUTPUT:
                    elem = new OperatorElement("Output "+ cellCount[m_selected]++, "output");
                    elem.addContent(new ParameterElement("Schema", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.TABLE:
                    elem = new OperatorElement("Table "+ cellCount[m_selected]++, "table");
                    elem.addContent(new ParameterElement("Schema", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_FILTER:
                    elem = new OperatorElement("Filter "+ cellCount[m_selected]++, "filter");
                    elem.addContent(new AttributeElement("Arg1", "", "", ""));
                    elem.addContent(new ParameterElement("Op", ""));
                    elem.addContent(new AttributeElement("Arg2", "", "", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_MAP:
                    elem = new OperatorElement("Map "+ cellCount[m_selected]++, "map");
                    cell = fact.createBox(elem);
                    break;
                //case EditorConstants.BOX_AGGREGATE:
                //    elem = new OperatorElement("Aggregate "+ cellCount[m_selected]++, "aggregate");
                //    elem.addContent(new ParameterElement("Parameter", ""));
                //    elem.addContent(new ParameterElement("Function", ""));
                //    cell = fact.createBox(elem);
                //    break;
                case EditorConstants.BOX_JOIN:
                    elem = new OperatorElement("Join "+ cellCount[m_selected]++, "join");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_UNION:
                    elem = new OperatorElement("Union "+ cellCount[m_selected]++, "union");
                    elem.addContent(new ParameterElement("Arg1", ""));
                    elem.addContent(new ParameterElement("Op", ""));
                    elem.addContent(new ParameterElement("Arg2", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_SELECT:
                    elem = new OperatorElement("Select "+ cellCount[m_selected]++, "select");
                    elem.addContent(new ParameterElement("Fields", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_INSERT:
                    elem = new OperatorElement("Insert "+ cellCount[m_selected]++, "insert");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_UPDATE:
                    elem = new OperatorElement("Update "+ cellCount[m_selected]++, "update");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_WAITFOR:
                    elem = new OperatorElement("Waitfor "+ cellCount[m_selected]++, "waitfor");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_RANDOMDROP:
                    elem = new OperatorElement("RandomDrop "+ cellCount[m_selected]++, "random_drop");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_WINDOWDROP:
                    elem = new OperatorElement("WindowDrop "+ cellCount[m_selected]++, "window_drop");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_USERDEFINED:
                    elem = new OperatorElement("UserDefined "+ cellCount[m_selected]++, "user_defined");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_CEP:
                    elem = new OperatorElement("CEP "+ cellCount[m_selected]++, "cep");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_GROUPBY:
                    elem = new OperatorElement("GroupBy "+ cellCount[m_selected]++, "groupby");
                    elem.addContent(new ParameterElement("Fields", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_SPLIT:
                    elem = new OperatorElement("Split "+ cellCount[m_selected]++, "split");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_WINDOW:
                    elem = new OperatorElement("Window "+ cellCount[m_selected]++, "window");
                    elem.addContent(new ParameterElement("Size", ""));
                    elem.addContent(new ParameterElement("Jump", ""));
                    elem.addContent(new ParameterElement("Emit size", ""));
                    elem.addContent(new ParameterElement("Size type", ""));
                    elem.addContent(new ParameterElement("Emit type", ""));
                    elem.addContent(new ParameterElement("External timestamp key", ""));
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_PRINT:
                    elem = new OperatorElement("Print "+ cellCount[m_selected]++, "print");
                    cell = fact.createBox(elem);
                    break;
                case EditorConstants.BOX_SINK:
                    elem = new OperatorElement("Sink "+ cellCount[m_selected]++, "sink");
                    cell = fact.createBox(elem);
                    break;
                default:
                    elem = null;
                    cell = null;
                    System.out.println("Can not figure out the cell type.");
                    System.exit(0);
            }

            JDialog diag = new JDialog(m_editorPanel.m_client, "Add New Operator To Borealis");
            GuiUtilities.centerOnParent(diag, true);

            Vector<BorealisElement> childElements = new Vector<>();

            childElements.add(elem);

            ElementTreeEditor elementEditor =
                    new ElementTreeEditor(m_editorPanel.m_borealisModel, elem, childElements);

            elementEditor.getComboBox().setEnabled(false);
            elementEditor.getComboBox().setVisible(false);

            elementEditor.getAddButton().setText("AddOperator");
            elementEditor.getAddButton().addActionListener
                    (
                            e1 -> {
                                diag.dispose();
                                m_editorPanel.getModel().add(elem, cell);
                                m_editorPanel.getNetworkTreeModel().showTreeUpdate();
                            }
                    );

            elementEditor.getRemoveButton().setVisible(false);

            diag.getContentPane().add(elementEditor);
            diag.setSize(new Dimension(400,100));
            diag.setVisible(true);
            m_editorPanel.getNetworkTreeModel().showTreeUpdate();
        }
    }
    
    
}






