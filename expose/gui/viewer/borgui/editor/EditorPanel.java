package borgui.editor;

import borgui.client.Client;
import borgui.common.DebugTrace;
import borgui.common.SmartScroller;
import borgui.common.SwingPanel;
import borgui.graph.*;
import borgui.visual.PlotColor;
import borgui.visual.ServerButtonPanel;
import borgui.xml.*;
import org.jdom2.Document;
import org.jdom2.Element;
import org.jgraph.event.GraphSelectionEvent;
import org.jgraph.event.GraphSelectionListener;

import javax.management.Query;
import javax.swing.*;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.text.DefaultCaret;
import javax.swing.tree.TreePath;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import com.github.underscore.U;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import com.fasterxml.jackson.dataformat.yaml.YAMLMapper;
import com.fasterxml.jackson.annotation.JsonView;
import org.json.JSONObject;
import org.json.XML;
import org.yaml.snakeyaml.Yaml;

class GuiUtilities {
    public static void centerOnParent(final Window child, final boolean absolute) {
        child.pack();
        boolean useChildsOwner = child.getOwner() != null ? ((child.getOwner() instanceof JFrame) || (child.getOwner() instanceof JDialog)) : false;
        final Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        final Dimension parentSize = useChildsOwner ? child.getOwner().getSize() : screenSize ;
        final Point parentLocationOnScreen = useChildsOwner ? child.getOwner().getLocationOnScreen() : new Point(0,0) ;
        final Dimension childSize = child.getSize();
        childSize.width = Math.min(childSize.width, screenSize.width);
        childSize.height = Math.min(childSize.height, screenSize.height);
        child.setSize(childSize);
        int x;
        int y;
        if ((child.getOwner() != null) && child.getOwner().isShowing()) {
            x = (parentSize.width - childSize.width) / 2;
            y = (parentSize.height - childSize.height) / 2;
            x += parentLocationOnScreen.x;
            y += parentLocationOnScreen.y;
        } else {
            x = (screenSize.width - childSize.width) / 2;
            y = (screenSize.height - childSize.height) / 2;
        }
        if (!absolute) {
            x /= 2;
            y /= 2;
        }
        child.setLocation(x, y);
    }
}

/**
 * A class that represent the editor panel to edit the digram.
 */
public class EditorPanel extends JPanel implements Editor
{
    protected Client              m_client;
    protected int                 m_select;
    protected ActionHandler       m_actionHandler;

    protected Toolbar             m_toolbar;
    protected XmlEditor           m_xmlEditor;
    protected XmlEditor           m_xmlPhysicalEditor;
    protected JScrollPane         m_editorView;
    protected JScrollPane         m_physicalEditorView;
    protected JTextArea           simulatorOutput;
    protected JPanel              centerPanel;
    protected JScrollPane         simulatorOutputScroll;
    protected ElementPanel        m_elementPanel;
    protected Process             runningSimulationProcess;
    protected ExperimentPanel     m_experimentPanel;
    protected ServerButtonPanel   m_serverButtonPanel = new ServerButtonPanel();
    public GraphButtonPanel       m_graphButtonPanel;
    protected GraphOverviewPanel  m_graphOverview;
    //protected GraphOverviewPanel  m_physicalGraphOverview;

    protected NetworkTreeEditor    m_networkTreeEditor;
    protected NetworkTreeModel    m_physicalNetworkTreeModel;
    protected BorealisElement     m_selectedElem;
    protected BorealisGraph       m_borealisGraph;
    //protected BorealisGraph       m_physicalGraph;
    protected BorealisModel       m_borealisModel;
    //protected BorealisModel       m_physicalModel;

    protected int                 m_newNameNumber = 1;


     ///////////////////////////////////////////////////////////////////////////
     //
     // Creates a new instance of EditorPanel.
     //
     public EditorPanel(Client client)
     {
     //.........................................................................


        super();

        m_client = client;
        client.editorPanel = this;

        m_actionHandler = new ActionHandler(this);

        ////////////////////////////////////////////////////////////////////////
        // tool bar
        //
        m_toolbar = new Toolbar();
        m_toolbar.setFloatable(false);

        ////////////////////////////////////////////////////////////////////////
        // toolbox tabbed pane
        //
        JPanel leftPanel = new JPanel();
        leftPanel.setLayout(new BoxLayout(leftPanel, BoxLayout.Y_AXIS));
         Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
         double width = screenSize.getWidth();
         double height = screenSize.getHeight();
        leftPanel.setPreferredSize(new Dimension((int)width/4, (int)height));

        m_graphButtonPanel = new GraphButtonPanel(this);
        leftPanel.add(m_graphButtonPanel);


        m_xmlEditor = new XmlEditor();

         m_xmlPhysicalEditor = new XmlEditor();

        m_borealisGraph = new BorealisGraph();
        m_borealisGraph.addGraphSelectionListener(new EditorGraphSelectionListener());
        m_editorView = new JScrollPane(m_borealisGraph);
        m_editorView = new JScrollPane(m_borealisGraph);
        m_editorView.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
        m_editorView.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        m_borealisModel = new BorealisModel(this,
                                            new BorealisDocument(),
                                            m_borealisGraph.getBorealisGraphModel(),
                                            m_borealisGraph);

        m_borealisModel.getDocument().setNetwork(new NetworkElement());

        ////////////////////////////////////////////////////////////////////////
        //
        m_elementPanel = new ElementPanel(m_borealisModel, true);
        m_elementPanel.getDeployTreeEditor().setServerButtonPanel(m_serverButtonPanel);

         m_networkTreeEditor = new NetworkTreeEditor(m_borealisModel);
         m_networkTreeEditor.m_networkTree.addTreeSelectionListener(new NetworkTreeListener());
         m_networkTreeEditor.m_networkTree.setCellRenderer(new NetworkTreeCellRenderer());

         JScrollPane treeScrollPane = new JScrollPane(m_networkTreeEditor.m_networkTree);
         treeScrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
         treeScrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
         leftPanel.add(treeScrollPane);
         JComponent rightAddPanel = new JPanel();

         rightAddPanel.setLayout(new BoxLayout(rightAddPanel, BoxLayout.X_AXIS));

         JButton addQueryButton = new JButton("Add Query");
         addQueryButton.setHorizontalAlignment(SwingConstants.LEFT);
         addQueryButton.setToolTipText("Add new query to Borealis");
         addQueryButton.addActionListener(new AddQueryButtonListener());
         rightAddPanel.add(addQueryButton);

         JButton addSchemaButton = new JButton("Add Schema");
         addSchemaButton.setHorizontalAlignment(SwingConstants.LEFT);
         addSchemaButton.setToolTipText("Add new schema to Borealis");
         addSchemaButton.addActionListener(new AddSchemaButtonListener());
         rightAddPanel.add(addSchemaButton);

         JButton addNodeButton = new JButton("Add Node");
         addNodeButton.setHorizontalAlignment(SwingConstants.LEFT);
         addNodeButton.setToolTipText("Add new node to simulation");
         addNodeButton.addActionListener(new AddNodeButtonListener());
         rightAddPanel.add(addNodeButton);

         JButton addDatasetButton = new JButton("Add dataset");
         addDatasetButton.setHorizontalAlignment(SwingConstants.LEFT);
         addDatasetButton.setToolTipText("Add new dataset to simulation");
         addDatasetButton.addActionListener(new AddDatasetButtonListener());
         rightAddPanel.add(addDatasetButton);

         JButton addExperimentButton = new JButton("Add experiment");
         addExperimentButton.setHorizontalAlignment(SwingConstants.LEFT);
         addExperimentButton.setToolTipText("Add new experiment to simulation");
         addExperimentButton.addActionListener(new AddExperimentButtonListener());
         rightAddPanel.add(addExperimentButton);

         JButton addPlotButton = new JButton("Add plot");
         addPlotButton.setHorizontalAlignment(SwingConstants.LEFT);
         addPlotButton.setToolTipText("Add new plot to create from simulation");
         addPlotButton.addActionListener(new AddPlotButtonListener());
         rightAddPanel.add(addPlotButton);

         leftPanel.add(rightAddPanel);

        //leftPanel.add(m_elementPanel);
        //BorealisGraph experimentGraph = new BorealisGraph();

        // BorealisModel experimentModel = new BorealisModel(this,
        //         new BorealisDocument(),
        //         experimentGraph.getBorealisGraphModel(),
        //         experimentGraph);
        // experimentModel.getDocument().setNetwork(new NetworkElement());
        //m_experimentPanel = new ExperimentPanel(experimentModel);


        ////////////////////////////////////////////////////////////////////////
        //
        JPanel rightPanel = new JPanel();
        rightPanel.setPreferredSize(new Dimension((int)width/4, (int)height));
        rightPanel.setLayout(new BoxLayout(rightPanel, BoxLayout.Y_AXIS));
        rightPanel.add(m_elementPanel);
        //rightPanel.add(m_experimentPanel);

        m_borealisGraph.setClient(m_client);
        m_borealisGraph.setBorealisModel(m_borealisModel);
        m_borealisGraph.setNetworkTreeModel(m_networkTreeEditor.m_networkTreeModel);
        m_borealisGraph.setDeploydoc(getDeployDoc());

        // experimentGraph.setClient(m_client);
        // experimentGraph.setBorealisModel(experimentModel);
        // experimentGraph.setDeploydoc(getDeployDoc());


        ////////////////////////////////////////////////////////////////////////
        // messages tabbed pane
        JTabbedPane messagesTabbedPane = new JTabbedPane();

        //
        JComponent panel3 = new JPanel();
        JTextPane jpt = new JTextPane();

        panel3.add(jpt);
        messagesTabbedPane.addTab("Typecheck Output", null, panel3, "");
        messagesTabbedPane.setMnemonicAt(0, KeyEvent.VK_1);

        //
        JComponent panel4 = new JPanel();
        messagesTabbedPane.addTab("Engine Output", null, panel4, "");
        messagesTabbedPane.setMnemonicAt(1, KeyEvent.VK_2);


        ////////////////////////////////////////////////////////////////////////
        //
        centerPanel = new JPanel();
        centerPanel.setLayout(new BoxLayout(centerPanel, BoxLayout.Y_AXIS));
        //centerPanel.add(m_editorView);


         simulatorOutput = new JTextArea (20, 128);
         DefaultCaret caret = (DefaultCaret)simulatorOutput.getCaret();
         caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);
         simulatorOutput.setEditable ( false ); // set textArea non-editable
         simulatorOutputScroll = new JScrollPane (simulatorOutput,
                 JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
         new SmartScroller( simulatorOutputScroll );
         //centerPanel.add(simulatorOutputScroll);

         JSplitPane splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,
                 m_editorView, simulatorOutputScroll);

         splitPane.setOneTouchExpandable(true);
         splitPane.setResizeWeight(0.5d);
         //splitPane.setDividerLocation(150);

         centerPanel.add(splitPane);

        this.setLayout(new BorderLayout());
        this.add(m_toolbar, BorderLayout.NORTH);
        this.add(leftPanel, BorderLayout.WEST);
        this.add(rightPanel, BorderLayout.EAST);
        this.add(centerPanel, BorderLayout.CENTER);

        JMenuItem menuItem;

        // add "File" menu options
        //
        menuItem = new JMenuItem("New Experiment");
        menuItem.addActionListener(m_actionHandler.getAction(ActionHandler.NEW_GRAPH_ACTION));
        m_client.addFileMenuItem(menuItem);

        menuItem = new JMenuItem("Load Experiment");
        menuItem.addActionListener(m_actionHandler.getAction(ActionHandler.LOAD_GRAPH_ACTION));
        m_client.addFileMenuItem(menuItem);

        menuItem = new JMenuItem("Save Experiment");
        menuItem.addActionListener(m_actionHandler.getAction(ActionHandler.SAVE_GRAPH_ACTION));
        m_client.addFileMenuItem(menuItem);

        menuItem = new JMenuItem("Quit");
        menuItem.addActionListener(m_actionHandler.getAction(ActionHandler.QUIT_ACTION));
        m_client.addFileMenuItem(menuItem);

        // add "View" menu options
        //
        menuItem = new JMenuItem("Diagram");
        menuItem.addActionListener(m_actionHandler.getAction(ActionHandler.VIEWDIAG_ACTION));
        m_client.addViewMenuItem(menuItem);

        menuItem = new JMenuItem("Xml");
        menuItem.addActionListener(m_actionHandler.getAction(ActionHandler.VIEWXML_ACTION));
        m_client.addViewMenuItem(menuItem);

        this.setVisible(true);
    }


    public DeployDocument getDeployDoc()
    {
        return m_elementPanel.getDeployTreeEditor().getDeployDoc();
    }


    public void setDeployDoc(DeployDocument deployDoc)
    {
        m_elementPanel.getDeployTreeEditor().setDeployDoc(deployDoc);
        //test
        m_borealisGraph.setDeploydoc(deployDoc);
        //m_physicalGraph.setDeploydoc(deployDoc);
    }


    public BorealisModel getModel()
    {
        return m_borealisModel;
    }


    public BorealisGraph getBorealisGraph()
    {
        return m_borealisGraph;
    }


    public NetworkTreeModel getNetworkTreeModel()
    {
        return m_networkTreeEditor.m_networkTreeModel;
    }


    public void setSelectedElement(BorealisElement element)
    {
        m_selectedElem = element;
        m_elementPanel.setElement(m_selectedElem);
        for (int i = 0; i < m_elementPanel.m_elementTreeEditor.m_elementTree.getRowCount(); i++) {
            m_elementPanel.m_elementTreeEditor.m_elementTree.expandRow(i);
        }
    }


    public void newGraph()
    {
        m_borealisModel.getDocument().setDocument(new BorealisDocument());

        try
        {   String  warning =
                BorealisModelXmlHandler.loadBorealisModel(m_borealisModel);

            if (!warning.equals(""))
            {   System.out.println("newGraph:  " + warning);
            }

            m_networkTreeEditor.m_networkTreeModel.showTreeUpdate();
            m_graphOverview.setGraphModel(m_borealisModel.getGraphModel());
        }
        catch (Exception  e)
        {   System.out.println("newGraph:  " + e.getMessage());
        }
    }

    public void doSave(File file) {
        // if a file was chosen to save
        if (file != null)
        {
            NetworkParameterElement networkParameterElement = m_borealisModel.getDocument().getNetworkParameters();
            Map<String, Map<String, String>> networkParameterMap = new HashMap<>();
            Map<String, String> bandwidth = new HashMap<>();
            bandwidth.put("migration", networkParameterElement.getContent(0).getValue());
            bandwidth.put("tuples", networkParameterElement.getContent(1).getValue());
            networkParameterMap.put("bandwidth", bandwidth);
            Map<String, String> latency = new HashMap<>();
            latency.put("migration", networkParameterElement.getContent(2).getValue());
            latency.put("tuples", networkParameterElement.getContent(3).getValue());
            networkParameterMap.put("latency", latency);

            List<Map<String, Object>> exportedExperiments = new ArrayList<>();
            for (ExperimentElement experimentElement : m_borealisModel.getDocument().getAllExperiments()) {
                TaskFlowSetElement rootTask = experimentElement.rootTaskElement;
                // Write to YAML
                List<Map<String, Object>> exportedExperimentList = ExperimentTaskElement.exportExperimentList(experimentElement.rootTaskElement);
                Map<String, Object> experiment = new HashMap<>();
                experiment.put("flow", exportedExperimentList);
                experiment.put("id", Integer.parseInt(experimentElement.getContent(0).getValue()));
                exportedExperiments.add(experiment);
            }

            List<Map<String, Object>> plots = new ArrayList<>();
            for (PlotElement plotElement : m_borealisModel.getDocument().getAllPlots()) {
                Map<String, Object> plotMap = new HashMap<>();
                plots.add(plotMap);
                plotMap.put("name", plotElement.getAttributeValue("name"));
                String type = plotElement.getAttributeValue("type");
                plotMap.put("type", type.replace(" ", "-"));
                plotMap.put("control-experiment-id", plotElement.getAttributeValue("Control-experiment-id"));
                if (type.equals("Migration time")) {
                    // Nothing to add
                } else if (type.equals("Input latency")) {
                    plotMap.put("query-id", plotElement.getAttributeValue("Query-id"));
                    plotMap.put("operator-name", plotElement.getAttributeValue("Operator-name"));
                    plotMap.put("stream-id", plotElement.getAttributeValue("Stream-id"));
                } else if (type.equals("Output latency")) {
                    plotMap.put("query-id", plotElement.getAttributeValue("Query-id"));
                    plotMap.put("operator-name", plotElement.getAttributeValue("Operator-name"));
                    plotMap.put("stream-id", plotElement.getAttributeValue("Stream-id"));
                } else {
                    System.out.println("Invalid plot type!");
                    System.exit(9);
                }
            }

            // Operators and queries
            BorealisDocument doc = m_borealisModel.getDocument();
            //List<Map<String, Object>> inputs = new ArrayList<>();
            List<Map<String, Object>> inputs = new ArrayList<>();
            List<Map<String, Object>> outputs = new ArrayList<>();
            List<Map<String, Object>> queries = new ArrayList<>();
            Map<String, List<Map<String, Object>>> applicationMap = new HashMap<>();
            applicationMap.put("inputs", inputs);
            applicationMap.put("outputs", outputs);
            applicationMap.put("queries", queries);

            for (QueryElement queryElement : (List<QueryElement>) doc.getAllQueries()) {
                Map<String, Object> queryMap = new HashMap<>();
                queryMap.put("id", Integer.parseInt(queryElement.getId()));
                queryMap.put("name", queryElement.getNameValue());
                List<Map<String, Object>> operators = new ArrayList<>();
                List<Map<String, Object>> edges = new ArrayList<>();
                Map<String, Map<String, Object>> streamIdToEdges = new HashMap<>();
                queryMap.put("operators", operators);
                queryMap.put("edges", edges);
                queries.add(queryMap);
                for (BoxElement boxElement : queryElement.getAllBoxes()) {
                    OperatorElement operatorElement = (OperatorElement) boxElement;
                    if (operatorElement.getType().equals("input")) {
                        for (OutElement outElement : (List<OutElement>) operatorElement.getAllOuts()) {
                            String streamId = outElement.getStream();
                            Map<String, Object> input = new HashMap<>();
                            //inputs.add(input);
                            input.put("name", operatorElement.getNameValue());
                            input.put("edge", outElement.getStream());
                            input.put("schema", outElement.getStream());
                            input.put("node", 0);
                            streamIdToEdges.put(streamId, new HashMap<>());
                            streamIdToEdges.get(streamId).put("stream", streamId);
                            streamIdToEdges.get(streamId).put("from", operatorElement.getNameValue());
                        }
                        //continue;
                    } else if (operatorElement.getType().equals("output")) {
                        for (InElement inElement : (List<InElement>) operatorElement.getAllIns()) {
                            String streamId = inElement.getStream();
                            Map<String, Object> output = new HashMap<>();
                            //outputs.add(output);
                            output.put("name", operatorElement.getNameValue());
                            output.put("edge", inElement.getStream());
                            output.put("schema", inElement.getStream());
                            output.put("node", 0);
                            streamIdToEdges.put(streamId, new HashMap<>());
                            streamIdToEdges.get(streamId).put("stream", streamId);
                            streamIdToEdges.get(streamId).put("to", operatorElement.getNameValue());

                            // Here, we add a final edge that simply has as "stream ID" the name of the operator, i.e., output
                            // DCEP-Sim needs this to create an operator
                            streamIdToEdges.put(operatorElement.getNameValue(), new HashMap<>());
                            Map<String, Object> final_output = new HashMap<>();
                            final_output.put("name", operatorElement.getNameValue());
                            final_output.put("type", "output");
                            streamIdToEdges.get(operatorElement.getNameValue()).put("from", final_output);
                            streamIdToEdges.get(operatorElement.getNameValue()).put("stream", operatorElement.getNameValue());
                        }
                        //continue;
                    } else {
                        for (InElement inElement : (List<InElement>) operatorElement.getAllIns()) {
                            String streamId = inElement.getStream();
                            Map<String, String> toMap = new HashMap<>();
                            toMap.put("type", "input");
                            toMap.put("name", operatorElement.getNameValue());
                            if (!streamIdToEdges.containsKey(streamId)) {
                                streamIdToEdges.put(streamId, new HashMap<>());
                            }
                            streamIdToEdges.get(streamId).put("to", toMap);
                            streamIdToEdges.get(streamId).put("stream", streamId);
                        }
                        for (OutElement outElement : (List<OutElement>) operatorElement.getAllOuts()) {
                            String streamId = outElement.getStream();
                            Map<String, String> fromMap = new HashMap<>();
                            fromMap.put("type", "output");
                            fromMap.put("name", operatorElement.getNameValue());
                            if (!streamIdToEdges.containsKey(streamId)) {
                                streamIdToEdges.put(streamId, new HashMap<>());
                            }
                            streamIdToEdges.get(streamId).put("from", fromMap);
                            streamIdToEdges.get(streamId).put("stream", streamId);
                        }
                    }
                    Map<String, Object> operatorMap = new HashMap<>();
                    String type = operatorElement.getType();
                    operatorMap.put("name", operatorElement.getNameValue());
                    operatorMap.put("type", operatorElement.getType());
                    Map<String, Object> parameters = new HashMap<>();
                    operatorMap.put("parameters", parameters);
                    List<ParameterElement> parameterElements = operatorElement.getAllParameters();
                    List<AttributeElement> attributeElements = operatorElement.getAllAttributes();
                    // Add parameters
                    if (type.equals("filter")) {
                        Map<String, String> arg1Map = new HashMap<>();
                        arg1Map.put("value", attributeElements.get(0).getValue());
                        arg1Map.put("value-type", attributeElements.get(0).getValueType());
                        arg1Map.put("type", attributeElements.get(0).getType());
                        parameters.put("arg1", arg1Map);
                        parameters.put("op", parameterElements.get(0).getValue());
                        Map<String, String> arg2Map = new HashMap<>();
                        arg2Map.put("value", attributeElements.get(1).getValue());
                        arg2Map.put("value-type", attributeElements.get(1).getValueType());
                        arg2Map.put("type", attributeElements.get(1).getType());
                        parameters.put("arg2", arg2Map);
                    } else if (type.equals("aggregation")) {
                        parameters.put("field", parameterElements.get(0).getValue());
                        parameters.put("aggregation-type", parameterElements.get(1).getValue());
                    } else if (type.equals("groupby")) {
                        List<String> groups = Arrays.stream(parameterElements.get(0).getValue().split(",")).collect(Collectors.toList());
                        parameters.put("fields", groups);
                    } else if (type.equals("select")) {
                        List<String> fields = Arrays.stream(parameterElements.get(0).getValue().split(",")).collect(Collectors.toList());
                        parameters.put("fields", fields);
                    } else if (type.equals("join")) {
                    } else if (type.equals("sink")) {
                    } else if (type.equals("print")) {
                    } else if (type.equals("cep")) {
                        parameters.put("pattern", parameterElements.get(0).getValue());
                    } else if (type.equals("input")) {
                        String streamName = ((OutElement) operatorElement.getAllOuts().get(0)).getStream();
                        int streamId = -1;
                        String alias = parameterElements.get(0).getValue();
                        for (SchemaElement schemaElement : (List<SchemaElement>) doc.getAllSchemas()) {
                            if (schemaElement.getNameValue().equals(streamName)) {
                                streamId = Integer.parseInt(schemaElement.getStreamId());
                                break;
                            }
                        }
                        parameters.put("stream-id", streamId);
                        parameters.put("alias", alias);
                    } else if (type.equals("output")) {
                        String streamName = ((InElement) operatorElement.getAllIns().get(0)).getStream();
                        int streamId = -1;
                        for (SchemaElement schemaElement : (List<SchemaElement>) doc.getAllSchemas()) {
                            if (schemaElement.getNameValue().equals(streamName)) {
                                streamId = Integer.parseInt(schemaElement.getStreamId());
                                break;
                            }
                        }
                        parameters.put("stream-id", streamId);
                    } else if (type.equals("split")) {
                    } else if (type.equals("window")) {
                        parameters.put("size", Integer.parseInt(parameterElements.get(0).getValue()));
                        parameters.put("jump", Integer.parseInt(parameterElements.get(1).getValue()));
                        parameters.put("emit-size", Integer.parseInt(parameterElements.get(2).getValue()));
                        parameters.put("size-type", parameterElements.get(3).getValue());
                        parameters.put("emit-type", parameterElements.get(4).getValue());
                        parameters.put("external-timestamp-field", parameterElements.get(5).getValue());
                    } else {
                        System.err.println("Invalid operator type " + type);
                        System.exit(0);
                    }
                    operators.add(operatorMap);
                }
                Map<String, String> streamNameToOperatorId = new HashMap<>();
                for (BoxElement boxElement : queryElement.getAllBoxes()) {
                    OperatorElement operatorElement = (OperatorElement) boxElement;
                    String name = operatorElement.getNameValue();

                    for (InElement inputElement : (List<InElement>) operatorElement.getAllIns()) {
                        Map<String, String> toMap = new HashMap<>();
                        String streamId = inputElement.getStream();
                        toMap.put("type", "intermediate");
                        toMap.put("name", name);
                        if (!streamIdToEdges.containsKey(streamId)) {
                            streamIdToEdges.put(streamId, new HashMap<>());
                        }
                        streamIdToEdges.get(streamId).put("to", toMap);
                        streamIdToEdges.get(streamId).put("stream", streamId);
                    }

                    for (OutElement outputElement : (List<OutElement>) operatorElement.getAllOuts()) {
                        Map<String, String> fromMap = new HashMap<>();
                        String streamId = outputElement.getStream();
                        fromMap.put("type", "intermediate");
                        fromMap.put("name", name);
                        if (!streamIdToEdges.containsKey(streamId)) {
                            streamIdToEdges.put(streamId, new HashMap<>());
                        }
                        streamIdToEdges.get(streamId).put("from", fromMap);
                        streamIdToEdges.get(streamId).put("stream", streamId);
                    }
                }

                for (Map<String, Object> edge : streamIdToEdges.values()) {
                    edges.add(edge);
                }
            }

            List<Map<String, Object>> datasetMapList = new ArrayList<>();
            List<DatasetElement> datasets = m_borealisModel.getDocument().getAllDatasets();
            for (int i = 0; i < datasets.size(); i++) {
                DatasetElement datasetElement = datasets.get(i);
                Map<String, Object> datasetMap = new HashMap<>();
                datasetMapList.add(datasetMap);
                datasetMap.put("name", datasetElement.getNameValue());
                datasetMap.put("id", Integer.parseInt(datasetElement.getAttributeValue("ID")));
                datasetMap.put("file", datasetElement.getAttributeValue("File"));
                datasetMap.put("type", datasetElement.getAttributeValue("Type"));
            }

            // Nodes (not 100% needed here)
            //this.m_physicalModel
            // Schemas
            List<Map<String, Object>> schemaMapList = new ArrayList<>();
            List<SchemaElement> schemas = m_borealisModel.getDocument().getAllSchemas();
            for (int i = 0; i < schemas.size(); i++) {
                SchemaElement schemaElement = schemas.get(i);
                Map<String, Object> schemaMap = new HashMap<>();
                schemaMapList.add(schemaMap);
                schemaMap.put("name", schemaElement.getNameValue());
                schemaMap.put("stream-id", Integer.parseInt(schemaElement.getStreamId()));
                List<Map<String, String>> fields = new ArrayList<>();
                for (FieldElement fieldElement : (List<FieldElement>) schemaElement.getAllFields()) {
                    Map<String, String> field = new HashMap<>();
                    field.put("name", fieldElement.getNameValue());
                    field.put("type", fieldElement.getType());
                    fields.add(field);
                }
                schemaMap.put("tuple-format", fields);
            }

            Map<String, Object> configuration = new HashMap<>();
            List<Map<String, Object>> experiments = new ArrayList<>(exportedExperiments);
            configuration.put("experiments", experiments);
            configuration.put("plots", plots);
            configuration.put("stream-definitions", schemaMapList);
            configuration.put("datasets", datasetMapList);
            configuration.put("spequeries", applicationMap);
            configuration.put("tracepoints", new ArrayList<>());
            configuration.put("network", networkParameterMap);
            Yaml yaml = new Yaml();
            String yamlString = yaml.dump(configuration);
            System.out.println("Dumped yaml:\n" + yamlString);

            try (PrintWriter out = new PrintWriter(file)) {
                out.println(yamlString);
            } catch (java.io.FileNotFoundException e) {
                e.printStackTrace();
            }
        }
    }

    public void saveGraph()
    {
        File file = SwingPanel.showFileDialog(m_client, "Save Graph");
        doSave(file);
    }

    public static String jsonToYaml(String jsonString) throws JsonProcessingException, IOException {
        // parse JSON
        JsonNode jsonNodeTree = new ObjectMapper().readTree(jsonString);
        // save it as YAML
        String jsonAsYaml = new YAMLMapper().writeValueAsString(jsonNodeTree);
        return jsonAsYaml;
    }

    public static String convertYamlToJson(String yaml) {
        ObjectMapper yamlReader = new ObjectMapper(new YAMLFactory());
        Object obj = null;
        try {
            obj = yamlReader.readValue(yaml, Object.class);
        } catch (JsonProcessingException e) {
            e.printStackTrace();
        }

        ObjectMapper jsonWriter = new ObjectMapper();
        try {
            return jsonWriter.writeValueAsString(obj);
        } catch (JsonProcessingException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static String convertJsonToXML(String jsonStr) {
        String xml = U.jsonToXml(jsonStr);
        return xml;
    }

    public static String convertXMLToJson(String xmlStr) {
        String json = U.xmlToJson(xmlStr);
        return json;
    }


    Map<String, Object> Loadyaml(String yamlFile) {
        FileInputStream fis = null;
        Yaml yaml = new Yaml();
        try {
            fis = new FileInputStream(yamlFile.replaceFirst("^~", System.getProperty("user.home")));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            System.exit(22);
        }
        Map<String, Object> map;
        synchronized (yaml) {
            map = yaml.load(fis);
        }

        return map;
    }


    ////////////////////////////////////////////////////////////////////////////
    //
    public void loadGraph()
    {
    //..........................................................................


        File  file = SwingPanel.showFileDialog(m_client, "Load Graph");

        if (file != null)
        {
            StringBuilder contentBuilder = new StringBuilder();
            try (Stream<String> stream = Files.lines(Paths.get(file.getAbsolutePath()), StandardCharsets.UTF_8)) {
                //Read the content with Stream
                stream.forEach(s -> contentBuilder.append(s).append("\n"));
            }
            catch (IOException e) {
                e.printStackTrace();
            }

            String yamlString = contentBuilder.toString();
            Map<String, Object> yamlConfiguration = Loadyaml(file.getAbsolutePath());

            BorealisDocument borealisDoc = new BorealisDocument();
            BorealisDocument physicalDoc = new BorealisDocument();

            Map<String, Map<String, String>> networkPerformance = (Map<String, Map<String, String>> ) yamlConfiguration.get("network");
            Map<String, String> bandwidth = networkPerformance.get("bandwidth");
            String bw_tuples = bandwidth.get("tuples");
            String bw_migration = bandwidth.get("migration");
            Map<String, String> latency = networkPerformance.get("latency");
            String lat_tuples = latency.get("tuples");
            String lat_migration = latency.get("migration");
            NetworkParameterElement networkParameterElement = new NetworkParameterElement(bw_migration, bw_tuples, lat_migration, lat_tuples);
            borealisDoc.setNetworkParameters(networkParameterElement);

            for (Map<String, Object> streamDefinition : (ArrayList<Map<String, Object> >) yamlConfiguration.get("stream-definitions")) {
                String streamName = (String) streamDefinition.get("name");
                String streamId = Integer.toString((int) streamDefinition.get("stream-id"));
                SchemaElement schemaElement = new SchemaElement(streamName, streamId);
                List<Map<String, String>> tupleFormat = (List<Map<String, String>>) streamDefinition.get("tuple-format");
                for (Map<String, String> attribute : tupleFormat) {
                    String attributeName = attribute.get("name");
                    String attributeType = attribute.get("type");
                    FieldElement fieldElement = new FieldElement(attributeName, attributeType);
                    schemaElement.addField(fieldElement);
                }
                borealisDoc.addSchema(schemaElement);
            }

            for (Map<String, Object> dataset : (ArrayList<Map<String, Object> >) yamlConfiguration.get("datasets")) {
                String name = (String) dataset.get("name");
                String id = Integer.toString((int) dataset.get("id"));
                String fileLocation = (String) dataset.get("file");
                //String streamId = Integer.toString((int) dataset.get("stream-id"));
                String type = (String) dataset.get("type");
                DatasetElement datasetElement = new DatasetElement(name, id, fileLocation, type);
                borealisDoc.addDataset(datasetElement);
            }

            List<Map<String, Object>> plots = (List<Map<String, Object>>)yamlConfiguration.getOrDefault("plots", new ArrayList<>());
            for (Map<String, Object> plot : plots) {
                String name = (String) plot.get("name");
                String type = ((String) plot.get("type")).replace("-", " ");

                if (type.equals("Migration time")) {
                    String control_experiment_id = (String) plot.getOrDefault("control-experiment-id", "");
                    PlotElement migrationTimePlotElement = new PlotElement(name, type);
                    migrationTimePlotElement.setAttribute("Control-experiment-id", control_experiment_id);
                    migrationTimePlotElement.index = 0;
                    borealisDoc.addPlot(migrationTimePlotElement);
                } else if (type.equals("Input latency")) {
                    String query_id = (String) plot.get("query-id");
                    String operator_name = (String) plot.get("operator-name");
                    String stream_id = (String) plot.get("stream-id");
                    String control_experiment_id = (String) plot.getOrDefault("control-experiment-id", "");
                    PlotElement inputLatencyPlotElement = new PlotElement(name, type);
                    inputLatencyPlotElement.setAttribute("Query-id", query_id);
                    inputLatencyPlotElement.setAttribute("Operator-name", operator_name);
                    inputLatencyPlotElement.setAttribute("Stream-id", stream_id);
                    inputLatencyPlotElement.setAttribute("Control-experiment-id", control_experiment_id);
                    inputLatencyPlotElement.index = 1;
                    borealisDoc.addPlot(inputLatencyPlotElement);
                } else if (type.equals("Output latency")) {
                    String query_id = (String) plot.get("query-id");
                    String operator_name = (String) plot.get("operator-name");
                    String stream_id = (String) plot.get("stream-id");
                    String control_experiment_id = (String) plot.get("control-experiment-id");
                    PlotElement outputLatencyPlotElement = new PlotElement(name, type);
                    outputLatencyPlotElement.setAttribute("Query-id", query_id);
                    outputLatencyPlotElement.setAttribute("Operator-name", operator_name);
                    outputLatencyPlotElement.setAttribute("Stream-id", stream_id);
                    outputLatencyPlotElement.setAttribute("Control-experiment-id", control_experiment_id);
                    outputLatencyPlotElement.index = 2;
                    borealisDoc.addPlot(outputLatencyPlotElement);
                } else {
                    System.out.println("Invalid plot type!");
                    System.exit(0);
                }
            }

            Map<String, Object> speQueries = (Map<String, Object>)yamlConfiguration.get("spequeries");

            for (Map<String, Object> query : (ArrayList<Map<String, Object> >) speQueries.get("queries")) {
                List<Map<String, Object>> operators = (List<Map<String, Object>>) query.get("operators");

                String queryName = (String) query.get("name");
                String queryId = Integer.toString((int) query.get("id"));
                QueryElement queryElement = new QueryElement(queryName, queryId);

                for (Map<String, Object> operator : operators) {
                    OperatorElement operatorElement = new OperatorElement((String) operator.get("name"), (String) operator.get("type"));
                    //borealisDoc.addBox(operatorElement, queryElement);

                    Map<String, Object> parameters = (Map<String, Object>) operator.get("parameters");
                    String operatorType = (String) operator.get("type");
                    if (operatorType.equals("filter")) {
                        Map<String, String> arg1Map = (Map<String, String>) parameters.get("arg1");
                        Map<String, String> arg2Map = (Map<String, String>) parameters.get("arg2");
                        operatorElement.addAttribute(new AttributeElement("Arg1", arg1Map.get("type"), arg1Map.get("value"), arg1Map.get("value-type")));
                        operatorElement.addParameter(new ParameterElement("Operator", (String) parameters.get("op")));
                        operatorElement.addAttribute(new AttributeElement("Arg2", arg2Map.get("type"), arg2Map.get("value"), arg2Map.get("value-type")));
                    } else if (operatorType.equals("aggregation")) {
                        operatorElement.addParameter(new ParameterElement("field", (String) parameters.get("field")));
                        operatorElement.addParameter(new ParameterElement("aggregation-type", (String) parameters.get("aggregation-type")));
                    } else if (operatorType.equals("groupby")) {
                        List<String> groups_lst = (List<String>) parameters.get("fields");
                        String groups_str = String.join(",", groups_lst);
                        operatorElement.addParameter(new ParameterElement("fields", groups_str));
                    } else if (operatorType.equals("select")) {
                        List<String> groups_lst = (List<String>) parameters.get("fields");
                        String groups_str = String.join(",", groups_lst);
                        operatorElement.addParameter(new ParameterElement("fields", groups_str));
                    } else if (operatorType.equals("join")) {
                    } else if (operatorType.equals("cep")) {
                        operatorElement.addParameter(new ParameterElement("Pattern", (String) parameters.get("pattern")));
                    } else if (operatorType.equals("split")) {

                    } else if (operatorType.equals("window")) {
                        operatorElement.addParameter(new ParameterElement("Size", Integer.toString((Integer) parameters.get("size"))));
                        operatorElement.addParameter(new ParameterElement("Jump", Integer.toString((Integer) parameters.get("jump"))));
                        operatorElement.addParameter(new ParameterElement("Emit size", Integer.toString((Integer) parameters.get("emit-size"))));
                        operatorElement.addParameter(new ParameterElement("Size type", (String) parameters.get("size-type")));
                        operatorElement.addParameter(new ParameterElement("Emit type", (String) parameters.get("emit-type")));
                        operatorElement.addParameter(new ParameterElement("External timestamp field", (String) parameters.get("external-timestamp-field")));
                    } else if (operatorType.equals("input")) {
                        System.out.println("new input operator");
                        operatorElement.addParameter(new ParameterElement("Alias", (String) parameters.get("alias")));
                    } else if (operatorType.equals("output")) {

                    } else if (operatorType.equals("sink")) {

                    } else if (operatorType.equals("print")) {

                    } else {
                        System.err.println("Invalid operator type " + operatorType);
                        System.exit(0);
                    }

                    queryElement.addBox(operatorElement);
                }
                List<Map<String, Object>> edges = (List<Map<String, Object>>) query.get("edges");
                for (Map<String, Object> edge : edges) {
                    Map<String, String> fromMap = (Map<String, String>) edge.get("from");
                    Map<String, String> toMap = (Map<String, String>) edge.get("to");
                    String fromName = fromMap.get("name");
                    //String fromType = fromMap.get("type");
                    String toName = null;
                    if (toMap != null) {
                        toName = toMap.get("name");
                    }
                    String streamId = (String) edge.get("stream");
                    //String toType = toMap.get("type");

                    for (BoxElement boxElement : queryElement.getAllBoxes()) {
                        OperatorElement operatorElement = (OperatorElement) boxElement;
                        String operatorName = operatorElement.getNameValue();
                        if (operatorName.equals(fromName)) {
                                for (BoxElement innerBoxElement : queryElement.getAllBoxes()) {
                                    OperatorElement innerOperatorElement = (OperatorElement) innerBoxElement;
                                    String innerOperatorName = innerOperatorElement.getNameValue();
                                    if (innerOperatorName.equals(toName)) {
                                        operatorElement.addOut(new OutElement(streamId));
                                        break;
                                    }
                                }
                            for (OutputElement innerOutputElement : queryElement.getOutputs()) {
                                String innerOutputName = innerOutputElement.getStream();
                                if (innerOutputName.equals(toName)) {
                                    operatorElement.addOut(new OutElement(streamId));
                                    break;
                                }
                            }
                        } else if (operatorName.equals(toName)) {
                                for (BoxElement innerBoxElement : queryElement.getAllBoxes()) {
                                    OperatorElement innerOperatorElement = (OperatorElement) innerBoxElement;
                                    String innerOperatorName = innerOperatorElement.getNameValue();
                                    if (innerOperatorName.equals(fromName)) {
                                        operatorElement.addIn(new InElement(streamId));
                                        break;
                                    }
                                }
                            for (InputElement innerInputElement : queryElement.getInputs()) {
                                String innerInputName = innerInputElement.getStream();
                                if (innerInputName.equals(fromName)) {
                                    operatorElement.addIn(new InElement(streamId));
                                    break;
                                }
                            }
                        }
                    }
                }

                for (BoxElement boxElement : queryElement.getAllBoxes()) {
                    List inputElements = boxElement.getAllIns();
                    List outputElements = boxElement.getAllOuts();
                    System.out.println();
                }


                borealisDoc.addQuery(queryElement);
            }
            for (Map<String, Object> experiment : (ArrayList<Map<String, Object> >) yamlConfiguration.get("experiments")) {
                int experiment_id = (int) experiment.get("id");
                ExperimentElement experimentElement = new ExperimentElement(m_elementPanel.m_elementTreeEditor.m_elementTreeModel.m_root, experiment_id);
                //borealisDoc.setExperiment(experimentElement);
                borealisDoc.addExperiment(experimentElement);
                Set<Integer> nodes = new HashSet<>();
                List<Map<String, Object> > cmds = (ArrayList<Map<String, Object> >) experiment.get("flow");
                PrepareTasks(experimentElement, null, cmds, nodes);
                for (int nodeId : nodes) {
                    SimulationNodeElement simulationNodeElement = new SimulationNodeElement("Node", nodeId);
                    physicalDoc.addSimulationNode(simulationNodeElement);
                    BoxElement elem = new BoxElement("Node " + nodeId, "");
                    QueryElement queryElement = new QueryElement();
                    physicalDoc.addBox(elem, queryElement);
                    physicalDoc.addQuery(queryElement);
                }
            }

            String  warning = null;
            this.m_borealisModel.getDocument().setDocument((BorealisDocument) borealisDoc.clone());
            //this.m_physicalModel.getDocument().setDocument((BorealisDocument) physicalDoc.clone());
            //try {
                warning = BorealisModelXmlHandler.loadBorealisModel(
                        m_borealisModel);
                //warning = BorealisModelXmlHandler.loadBorealisModel(
                //        m_physicalModel);
            /*} catch (Exception e) {
                throw new RuntimeException(e);
            }*/

            if (!warning.equals(""))
                    {   DebugTrace.trace(warning);

                        JOptionPane.showConfirmDialog(null, warning,
                                "XML parse warning",
                                JOptionPane.CLOSED_OPTION,
                                JOptionPane.WARNING_MESSAGE);
                    }

                    //m_graphOverview.setGraphModel(
                    //        m_borealisModel.getGraphModel());
                    //m_physicalGraphOverview.setGraphModel(
                    //        m_physicalModel.getGraphModel());

                    m_elementPanel.setFocusTabDeploy();

                    Vector<String>  ipPorts = borealisDoc.getAllDistinctIpPortsString();
                    getDeployDoc().addNodes(ipPorts);

                    Vector<String>  newIpPorts = getDeployDoc().getIpPorts();
                    PlotColor plotColor = new PlotColor(newIpPorts);

                    m_serverButtonPanel.setPlotColor(plotColor);
                    m_serverButtonPanel.setServerIpPort(newIpPorts);
                    //m_serverButtonPanel.getParent().getParent().getParent().repaint();

                    m_borealisGraph.setDeploydoc(getDeployDoc());
                    //m_physicalGraph.setDeploydoc(getDeployDoc());

                    //  A default deployment needs to be applied here???
                    m_borealisGraph.changeNodeColor(plotColor);

                    m_elementPanel.showTreeUpdate();
                    m_networkTreeEditor.showTreeUpdate();
                    //m_physicalNetworkTreeModel.showTreeUpdate();
                    m_elementPanel.m_elementTreeEditor.m_elementTreeModel.showTreeUpdate();
                //}
        }

        for (int i = 0; i < m_elementPanel.m_elementTreeEditor.m_elementTree.getRowCount(); i++) {
            m_elementPanel.m_elementTreeEditor.m_elementTree.expandRow(i);
        }
    }

    private class SimulationBatch {
         List<ExperimentElement> experiments;
         String yaml_path;

         SimulationBatch(List<ExperimentElement> experiments, String yaml_path) {
             this.experiments = experiments;
             this.yaml_path = yaml_path;
         }

         public void processOutput(BufferedReader reader) {
             StringBuilder builder = new StringBuilder();
             String line;
             while (true) {
                 try {
                     if ((line = reader.readLine()) == null) break;
                 } catch (IOException e) {
                     e.printStackTrace();
                     break;
                 }
                /*synchronized (simulatorOutput) {
                    simulatorOutput.append(line + "\n");
                }*/
                 builder.append(line);
                 builder.append(System.getProperty("line.separator"));
                 try {
                     Thread.sleep(1);
                 } catch (InterruptedException e) {
                     e.printStackTrace();
                 }
             }
         }

         public void analyzeSimulations() {
             System.out.println("Now running analyses");
             for (PlotElement plotElement : m_borealisModel.getDocument().getAllPlots()) {
                 System.out.println("Creating plot " + plotElement.getNameValue());
                 System.out.println("Type: " + plotElement.type);
             }

             System.out.println("Done with analysis");
             // TODO: Run the analysis tool from scripts/analyze_simulations.py, which reads results from expose/Results/
             String script_path = "../../../scripts/analyze_simulations.py";
             ProcessBuilder processBuilder = new ProcessBuilder("python3", script_path + " " + yaml_path);
             processBuilder.directory(new File("../../../"));
             Process analysisProcess = null;
             try {
                 analysisProcess = processBuilder.start();
             } catch (IOException e) {
                 e.printStackTrace();
                 System.exit(0);
             }
             //p.getOutputStream()
             //BufferedReader reader3 = new BufferedReader(new InputStreamReader(p.get()));
             BufferedReader reader2 = new BufferedReader(new InputStreamReader(analysisProcess.getErrorStream()));
             BufferedReader reader = new BufferedReader(new InputStreamReader(analysisProcess.getInputStream()));
             Thread t1 = new Thread(() -> processOutput(reader));
             Thread t2 = new Thread(() -> processOutput(reader2));
             t1.start();
             t2.start();
             try {
                 t1.join();
                 t2.join();
             } catch (InterruptedException e) {
                 e.printStackTrace();
                 System.exit(10);
             }

             m_client.analysisPanel.showFigures();
         }

         public void runNextSimulation() {
             if (experiments.isEmpty()) {
                 System.out.println("All experiments finished!");
                 analyzeSimulations();
                 return;
             }

             ExperimentElement experiment = experiments.get(0);
             experiments.remove(0);
             ProcessBuilder processBuilder = new ProcessBuilder("python3", "ns3", "run", "dcep-expose --yaml-config " + yaml_path + " --experiment-id " + experiment.experiment_id);
             processBuilder.directory(new File("../../../"));
             runningSimulationProcess = null;
             try {
                 runningSimulationProcess = processBuilder.start();
             } catch (IOException e) {
                 e.printStackTrace();
                 System.exit(0);
             }
             //p.getOutputStream()
             //BufferedReader reader3 = new BufferedReader(new InputStreamReader(p.get()));
             BufferedReader reader2 = new BufferedReader(new InputStreamReader(runningSimulationProcess.getErrorStream()));
             BufferedReader reader = new BufferedReader(new InputStreamReader(runningSimulationProcess.getInputStream()));
             new Thread(() -> processOutput(reader)).start();
             new Thread(() -> processOutput(reader2)).start();
         }
    }

    public void runSimulations() {
        try {
            Files.createDirectories(Paths.get(System.getProperty("java.io.tmpdir") + "/DCEP-Sim/"));
        } catch (IOException e) {
            e.printStackTrace();
        }
        String yaml_path = System.getProperty("java.io.tmpdir") + "/DCEP-Sim/" + UUID.randomUUID() + ".yaml";
        // Save current configuration to /tmp/expose-gui/... and run it with DCEP-Sim.
        File f = new File(yaml_path);
        doSave(f);
        SimulationBatch simulationBatch = new SimulationBatch(m_borealisModel.getDocument().getAllExperiments(), yaml_path);
        simulationBatch.runNextSimulation();
    }


    public void stopSimulation() {
        if (runningSimulationProcess != null && runningSimulationProcess.isAlive()) {
            //runningSimulationProcess.destroyForcibly();
            runningSimulationProcess.destroy();
            simulatorOutput.append("\n\n--------------Stopped simulation--------------\n\n");
        }
    }


    public void PrepareTasks(ExperimentElement experimentElement, ExperimentTaskElement parentTaskElement, List<Map<String, Object>> cmds, Set<Integer> nodes) {
        for (Map<String, Object> task : cmds) {
            int nodeId = 0;
            if (!task.get("node").equals("coordinator")) {
                nodeId = (Integer) task.get("node");
            }
            String cmd = (String) task.get("task");
            List<Object> args = (List<Object>) task.get("arguments");
            ExperimentTaskElement taskElement = new ExperimentTaskElement(cmd, args);
            taskElement.setNode(nodeId);
            taskElement.buildElementFromTask();
            if (parentTaskElement == null) {
                //parentTaskElement.addTask(taskElement);
                m_elementPanel.m_elementTreeEditor.m_elementTreeModel.insertNodeInto(taskElement, experimentElement.rootTaskElement, experimentElement.rootTaskElement.getContentSize());
            } else if (parentTaskElement.getName().equals("loopTasks")) {
                TaskFlowSetElement flowTaskElement = (TaskFlowSetElement) parentTaskElement.getChildren().get(0);
                m_elementPanel.m_elementTreeEditor.m_elementTreeModel.insertNodeInto(taskElement, flowTaskElement, flowTaskElement.getContentSize());
            } else if (parentTaskElement.getName().equals("batchTasks")) {
                TaskFlowSetElement flowTaskElement = (TaskFlowSetElement) parentTaskElement.getChildren().get(1);
                m_elementPanel.m_elementTreeEditor.m_elementTreeModel.insertNodeInto(taskElement, flowTaskElement, flowTaskElement.getContentSize());
            } else {
                //parentTaskElement.addTask(taskElement);
                m_elementPanel.m_elementTreeEditor.m_elementTreeModel.insertNodeInto(taskElement, parentTaskElement, parentTaskElement.getContentSize());
            }
            for (int i = 0; i < m_elementPanel.m_elementTreeEditor.m_elementTree.getRowCount(); i++) {
                m_elementPanel.m_elementTreeEditor.m_elementTree.expandRow(i);
            }
            if (cmd.equals("loopTasks")) {
                List<Map<String, Object>> loopCmds = (ArrayList<Map<String, Object>>) args.get(1);
                PrepareTasks(experimentElement, taskElement, loopCmds, nodes);
            } else if (cmd.equals("batchTasks")) {
                List<Map<String, Object>> batchCmds = (ArrayList<Map<String, Object>>) args.get(0);
                PrepareTasks(experimentElement, taskElement, batchCmds, nodes);
            } else {

            }
        }
    }


    public void newDeploy()
    {
        setDeployDoc(new DeployDocument());
        m_serverButtonPanel.removeAllServers();
        //m_elementPanel.getDeployXmlTreeEditor().setServerButtonPanel(m_serverButtonPanel);
        m_elementPanel.setFocusTabDeploy();
        //m_borealisGraph.setDeploydoc(getDeployDoc());
        m_borealisModel.getDocument().clearAllNodesAttribute();
        m_borealisGraph.clearNodesColor();
    }


    public void saveDeploy()
    {
        File file = SwingPanel.showFileDialog(m_client, "Save Deploy XML");

        // if a file was chosen to save
        if (file != null)
        {
            try
            {
                XmlHandler.docToXml(getDeployDoc(), new FileOutputStream(file));
            }
            catch(java.io.FileNotFoundException e)
            {
                DebugTrace.trace(e.getMessage());
            }
        }

        m_elementPanel.setFocusTabDeploy();
    }



    ////////////////////////////////////////////////////////////////////////////
    //
    //  Load a deployment XML file.
    //
    public void loadDeploy()
    {
    //..........................................................................


        File file = SwingPanel.showFileDialog(m_client, "Load Deployment XML");

        if (file != null)            // if a file was chosen to load
        {   try
            {   Document  doc = XmlHandler.xmlToDoc(file);

                if (!doc.getRootElement().getName().equals(
                                              XmlConstants.DEPLOY_ELEMENT))
                {
                    JOptionPane.showConfirmDialog(null,
                            "The file you choosed is not a deploy XML file.",
                            "Not Deploy XML", JOptionPane.CLOSED_OPTION,
                            JOptionPane.WARNING_MESSAGE);
                }
                else
                {   setDeployDoc(new DeployDocument(doc));

                    BorealisModelXmlHandler.applyDeployment(m_borealisModel,
                                                            getDeployDoc());

                    Vector<String>  ipPorts = m_borealisModel.getDocument().
                                        getAllDistinctIpPortsString();

                    getDeployDoc().addNodes(ipPorts);

                    Vector<String> newIpPorts = getDeployDoc().getIpPorts();

                    PlotColor  plotColor = new PlotColor(newIpPorts);
                    m_serverButtonPanel.setPlotColor(plotColor);

                    m_serverButtonPanel.setServerIpPort(newIpPorts);
                    m_serverButtonPanel.getParent().getParent().getParent().repaint();

                    m_borealisGraph.setDeploydoc(getDeployDoc());
                    m_borealisGraph.changeNodeColor(plotColor);

                    m_elementPanel.showTreeUpdate();
                    m_networkTreeEditor.m_networkTreeModel.showTreeUpdate();
                }
            }
            catch (IOException e)
            {   DebugTrace.trace(e.getMessage());

                JOptionPane.showConfirmDialog(null,
                                              e.getMessage(),
                                              "Could not open an XML file.",
                                              JOptionPane.CLOSED_OPTION,
                                              JOptionPane.ERROR_MESSAGE);
            }
            catch (Exception e)
            {   DebugTrace.trace(e.getMessage());

                JOptionPane.showConfirmDialog(null,
                                              e.getMessage(),
                                              "XML parse error",
                                              JOptionPane.CLOSED_OPTION,
                                              JOptionPane.ERROR_MESSAGE);
            }
        }

        m_elementPanel.setFocusTabDeploy();
    }


    public void showDiagView()
    {
        m_editorView.setViewportView(m_borealisGraph);
        //m_physicalEditorView.setViewportView(m_physicalGraph);
    }


    public static String convertXmlToJson(String xmlStr) {
        String json = U.xmlToJson(xmlStr);
        return json;
    }


    public void showXmlView()
    {
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        //ByteArrayOutputStream stream2 = new ByteArrayOutputStream();

        XmlHandler.docToXml(m_borealisModel.getDocument(), stream);
        //XmlHandler.docToXml(m_physicalModel.getDocument(), stream2);

        String xml1 = stream.toString();
        //String xml2 = stream2.toString();
        m_xmlEditor.setText(convertXmlToJson(xml1));
        //m_xmlPhysicalEditor.setText(convertXmlToJson(xml2));
        m_editorView.setViewportView(m_xmlEditor);
        //m_physicalEditorView.setViewportView(m_xmlPhysicalEditor);
    }


    public void setSelection(int type)
    {
        m_select = type;
    }


    private class EditorGraphSelectionListener
       implements GraphSelectionListener
    {
        public void valueChanged(GraphSelectionEvent e)
        {
            BorealisElement element = null;
            BorealisModel model = getModel();

            Object[] selectedCells = e.getCells();

            for(int i = 0; i < selectedCells.length && element == null; i++)
            {
                Object cell = selectedCells[i];

                // if a graph cell
                if (cell instanceof BorealisGraphCell)
                {
                    BorealisGraphCell graphCell = (BorealisGraphCell) cell;
                    element = model.getElement(graphCell);
                }
            }

            if (element != null)
            {
                setSelectedElement(element);
            }

            boolean enabled = !m_borealisGraph.isSelectionEmpty();
            m_toolbar.removeBut.setEnabled(enabled);
        }
    }


    protected class Toolbar extends JToolBar
    {
        // private JComboBox zoom;
        //
        public JButton undoBut;
        public JButton redoBut;
        public JButton removeBut;

        /**
         * Constructs a new Toolbar object
         */
        public Toolbar()
        {
            super();

            // create buttons
            //
            JButton newGraphBut = new JButton(new ImageIcon("images/new_Graph.gif"));
            JButton saveGraphBut = new JButton(new ImageIcon("images/save_Graph.gif"));
            JButton loadGraphBut = new JButton(new ImageIcon("images/open_experiment.png"));
            JButton runSimulationBut = new JButton(new ImageIcon("images/play.png"));
            JButton stopSimulationBut = new JButton(new ImageIcon("images/stop.png"));

            JButton newDeployBut = new JButton(new ImageIcon("images/new_Deploy.gif"));
            JButton saveDeployBut = new JButton(new ImageIcon("images/save_Deploy.gif"));
            JButton loadDeployBut = new JButton(new ImageIcon("images/load_Deploy.gif"));



            JButton orgagniazeBut = new JButton(new ImageIcon("images/refresh.gif"));
            removeBut = new JButton(new ImageIcon("images/delete.gif"));
            JButton zoomBut = new JButton(new ImageIcon("images/zoom.gif"));
            JButton zoominBut = new JButton(new ImageIcon("images/zoomin.gif"));
            JButton zoomoutBut = new JButton(new ImageIcon("images/zoomout.gif"));


            // set tooltip text
            //
            newGraphBut.setToolTipText("Create a new query network graph");
            saveGraphBut.setToolTipText("Save the query network graph");
            loadGraphBut.setToolTipText("Load a query network graph");

            newDeployBut.setToolTipText("Create a new deployment XML");
            saveDeployBut.setToolTipText("Save the deployment XML");
            loadDeployBut.setToolTipText("Load a deployment XML");

            orgagniazeBut.setToolTipText("Organize the graph");
            removeBut.setToolTipText("Remove");
            removeBut.setEnabled(false);

            // add action listeners
            //
            newGraphBut.addActionListener(m_actionHandler.getAction(ActionHandler.NEW_GRAPH_ACTION));
            saveGraphBut.addActionListener(m_actionHandler.getAction(ActionHandler.SAVE_GRAPH_ACTION));
            loadGraphBut.addActionListener(m_actionHandler.getAction(ActionHandler.LOAD_GRAPH_ACTION));
            runSimulationBut.addActionListener(m_actionHandler.getAction(ActionHandler.RUN_SIMULATION_ACTION));
            stopSimulationBut.addActionListener(m_actionHandler.getAction(ActionHandler.STOP_SIMULATION_ACTION));

            newDeployBut.addActionListener(m_actionHandler.getAction(ActionHandler.NEW_DEPLOY_ACTION));
            saveDeployBut.addActionListener(m_actionHandler.getAction(ActionHandler.SAVE_DEPLOY_ACTION));
            loadDeployBut.addActionListener(m_actionHandler.getAction(ActionHandler.LOAD_DEPLOY_ACTION));

            removeBut.addActionListener(m_actionHandler.getAction(ActionHandler.REMOVE_ACTION));
            orgagniazeBut.addActionListener(m_actionHandler.getAction(ActionHandler.ORGAGNIZE_ACTION));
            zoomBut.addActionListener(m_actionHandler.getAction(ActionHandler.ZOOM_ACTION));
            zoominBut.addActionListener(m_actionHandler.getAction(ActionHandler.ZOOMIN_ACTION));
            zoomoutBut.addActionListener(m_actionHandler.getAction(ActionHandler.ZOOMOUT_ACTION));

            /*  NYI
            undoBut = new JButton(new ImageIcon("images/undo.gif"));
            redoBut = new JButton(new ImageIcon("images/redo.gif"));
            undoBut.setToolTipText("Undo");
            undoBut.setEnabled(false);
            redoBut.setToolTipText("Redo");
            redoBut.setEnabled(false);
            undoBut.addActionListener(m_actionHandler.getAction(ActionHandler.UNDO_ACTION));
            redoBut.addActionListener(m_actionHandler.getAction(ActionHandler.REDO_ACTION));
             */

            // filler panel
            //
            JPanel filler = new JPanel();

            // radio buttons
            //
            JRadioButton diagBut = new JRadioButton("Diag");
            JRadioButton xmlBut = new JRadioButton("Xml");
            diagBut.setSelected(true);

            // add to but group
            //
            ButtonGroup editorButGroup = new ButtonGroup();
            editorButGroup.add(diagBut);
            editorButGroup.add(xmlBut);

            diagBut.addActionListener(m_actionHandler.getAction(ActionHandler.VIEWDIAG_ACTION));
            xmlBut.addActionListener(m_actionHandler.getAction(ActionHandler.VIEWXML_ACTION));

            /*
            String[] zoomStrs = { "200%", "150%", "100%", "90%", "80%", "70%",
                  "60%", "50%", "10%" };
            zoom = new JComboBox(zoomStrs);
            zoom.setSelectedItem("100%");
            zoom.addItemListener(new ZoomListener(zoom, this));
            add(zoom);
            */

            // add all components to toolbar
            //
            add(newGraphBut);
            add(loadGraphBut);
            add(saveGraphBut);
            add(runSimulationBut);
            add(stopSimulationBut);

            addSeparator();

            add(newDeployBut);
            add(loadDeployBut);
            add(saveDeployBut);
            /*
            add(undoBut);
            add(redoBut);
             */

            addSeparator();

            add(removeBut);
            add(orgagniazeBut);

            addSeparator();

            add(zoomBut);
            add(zoominBut);
            add(zoomoutBut);
            add(filler);
            add(diagBut);
            add(xmlBut);
        }
    }


    // Update Undo/Redo Button State based on Undo Manager
    //
    protected void updateHistoryButtons()
    {
        // The View Argument Defines the Context
        //temp
        /*
        m_toolbar.undoBut.setEnabled(m_borealisModel.getUndoManager().canUndo(m_borealisGraph.getGraphLayoutCache()));
        m_toolbar.redoBut.setEnabled(m_borealisModel.getUndoManager().canRedo(m_borealisGraph.getGraphLayoutCache()));
         */
    }


    private class AddSchemaButtonListener implements ActionListener {
        public AddSchemaButtonListener() {
        }

        public void actionPerformed(ActionEvent e) {
            final SchemaElement element = new SchemaElement("NewName" + m_newNameNumber++);

            JDialog diag = new JDialog(m_client, "Add New Schema To Borealis");
            GuiUtilities.centerOnParent(diag, true);

            Vector<FieldElement> childElements = new Vector<FieldElement>();

            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_BYTE_TYPE));
            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_SHORT_TYPE));
            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_INT_TYPE));
            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_LONG_TYPE));
            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_SINGLE_TYPE));
            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_DOUBLE_TYPE));
            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_STRING_TYPE, 20));
            childElements.add(new FieldElement("field_name", XmlConstants.SCHEMA_TIMESTAMP_TYPE));

            ElementTreeEditor elementEditor = new ElementTreeEditor(
                    m_borealisModel, element, childElements);

            elementEditor.m_addCombo.removeAllItems();
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_BYTE_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_SHORT_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_INT_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_LONG_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_SINGLE_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_DOUBLE_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_STRING_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_TIMESTAMP_TYPE + " )");
            elementEditor.m_addCombo.setSelectedIndex(0);

            elementEditor.getAddButton().setText("Add Field");
            elementEditor.getAddButton().setActionCommand("AddField");
            elementEditor.getRemoveButton().setActionCommand("RemoveField");

            JButton addSchemaButton = new JButton("Add Schema");

            addSchemaButton.addActionListener
                    (
                            e1 -> {
                                if (element.getParent() == null) {
                                    if (m_borealisModel.getDocument().getAllSchemaNames().contains(
                                            element.getNameValue())) {
                                        JOptionPane.showConfirmDialog(null,
                                                "The schema name has already been used. Please change the name.",
                                                "Change Schema Name", JOptionPane.CLOSED_OPTION,
                                                JOptionPane.ERROR_MESSAGE);
                                    } else {
                                        m_borealisModel.getDocument().addSchema(
                                                (SchemaElement) element);

                                        diag.dispose();
                                    }
                                }
                            }
                    );

            elementEditor.getOptionPanel().add(addSchemaButton);

            diag.getContentPane().add(elementEditor);
            diag.setSize(new Dimension(500, 200));
            diag.setVisible(true);
            m_networkTreeEditor.m_networkTreeModel.showTreeUpdate();
        }
    }

    private class AddDatasetButtonListener implements ActionListener
    {
        int dataset_id = -1;
        public AddDatasetButtonListener()  {}

        // Center on parent ( absolute true/false (exact center or 25% upper left) )
        public void centerOnParent(final Window child, final boolean absolute) {
            child.pack();
            boolean useChildsOwner = child.getOwner() != null ? ((child.getOwner() instanceof JFrame) || (child.getOwner() instanceof JDialog)) : false;
            final Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            final Dimension parentSize = useChildsOwner ? child.getOwner().getSize() : screenSize ;
            final Point parentLocationOnScreen = useChildsOwner ? child.getOwner().getLocationOnScreen() : new Point(0,0) ;
            final Dimension childSize = child.getSize();
            childSize.width = Math.min(childSize.width, screenSize.width);
            childSize.height = Math.min(childSize.height, screenSize.height);
            child.setSize(childSize);
            int x;
            int y;
            if ((child.getOwner() != null) && child.getOwner().isShowing()) {
                x = (parentSize.width - childSize.width) / 2;
                y = (parentSize.height - childSize.height) / 2;
                x += parentLocationOnScreen.x;
                y += parentLocationOnScreen.y;
            } else {
                x = (screenSize.width - childSize.width) / 2;
                y = (screenSize.height - childSize.height) / 2;
            }
            if (!absolute) {
                x /= 2;
                y /= 2;
            }
            child.setLocation(x, y);
        }

        public void actionPerformed(ActionEvent e)
        {
            final DatasetElement element = new DatasetElement("Dataset", "" + m_newNameNumber++, "", "");

            JDialog diag = new JDialog(m_client, "Add New Node To Simulation");
            GuiUtilities.centerOnParent(diag, true);
            JButton addDatasetButton = new JButton("Add dataset");

            Vector<FieldElement> childElements = new Vector<>();

            childElements.add(new FieldElement("Dataset Id",XmlConstants.SCHEMA_BYTE_TYPE));

            ElementTreeEditor elementEditor = new ElementTreeEditor(
                    m_borealisModel, element, childElements);
            elementEditor.getAddButton().setActionCommand("AddDataset");

            addDatasetButton.addActionListener
                    (
                        e1 -> {
                                m_borealisModel.getDocument().addDataset(element);
                                diag.dispose();
                        }
                    );

            elementEditor.getOptionPanel().add(addDatasetButton);

            diag.getContentPane().add(elementEditor);
            centerOnParent(diag, true);
            diag.setSize(new Dimension(500,200));
            diag.setVisible(true);
        }
    }

    private class AddExperimentButtonListener implements ActionListener
    {
        int experiment_id = -1;
        public AddExperimentButtonListener()  {}

        // Center on parent ( absolute true/false (exact center or 25% upper left) )
        public void centerOnParent(final Window child, final boolean absolute) {
            child.pack();
            boolean useChildsOwner = child.getOwner() != null ? ((child.getOwner() instanceof JFrame) || (child.getOwner() instanceof JDialog)) : false;
            final Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            final Dimension parentSize = useChildsOwner ? child.getOwner().getSize() : screenSize ;
            final Point parentLocationOnScreen = useChildsOwner ? child.getOwner().getLocationOnScreen() : new Point(0,0) ;
            final Dimension childSize = child.getSize();
            childSize.width = Math.min(childSize.width, screenSize.width);
            childSize.height = Math.min(childSize.height, screenSize.height);
            child.setSize(childSize);
            int x;
            int y;
            if ((child.getOwner() != null) && child.getOwner().isShowing()) {
                x = (parentSize.width - childSize.width) / 2;
                y = (parentSize.height - childSize.height) / 2;
                x += parentLocationOnScreen.x;
                y += parentLocationOnScreen.y;
            } else {
                x = (screenSize.width - childSize.width) / 2;
                y = (screenSize.height - childSize.height) / 2;
            }
            if (!absolute) {
                x /= 2;
                y /= 2;
            }
            child.setLocation(x, y);
        }

        public void actionPerformed(ActionEvent e)
        {
            final ExperimentElement element = new ExperimentElement(new TaskFlowSetElement(), 1);

            JDialog diag = new JDialog(m_client, "Add New Experiment To Simulation");
            GuiUtilities.centerOnParent(diag, true);
            JButton addExperimentButton = new JButton("Add experiment");


            ElementTreeEditor elementEditor = new ElementTreeEditor(
                    m_borealisModel, element, new Vector<>());

            //elementEditor.m_addButton.addActionListener(new AddExperimentTaskButtonListener(elementEditor));

            elementEditor.m_addElements.add(new ExperimentTaskElement("deployQueries", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("addPotentialHost", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("addSourceNode", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("addSinkNode", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("sendDsAsSpecificStream", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("sendNRowsDsAsSpecificStream", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("disconnectNode", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("retEndOfStream", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("wait", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("loopTasks", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("batchTasks", new ArrayList<>()));
            elementEditor.m_addElements.add(new ExperimentTaskElement("adapt", new ArrayList<>()));

            for(Object option : elementEditor.m_addElements) {
                ExperimentTaskElement ete = (ExperimentTaskElement) option;
                elementEditor.m_addCombo.addItem(ete.getName());
            }
            elementEditor.m_addCombo.setSelectedIndex(0);
            elementEditor.getAddButton().setActionCommand("AddExperiment");

            addExperimentButton.addActionListener
                    (
                            e1 -> {
                                m_borealisModel.getDocument().addExperiment(element);
                                diag.dispose();
                            }
                    );

            elementEditor.add(addExperimentButton);

            diag.getContentPane().add(elementEditor);
            centerOnParent(diag, true);
            diag.setSize(new Dimension(500,200));
            diag.setVisible(true);
        }
    }

    private class PlotDropMenuActionListener implements ActionListener {
        ElementTreeEditor elementEditor;

        PlotDropMenuActionListener(ElementTreeEditor elementEditor) {
            this.elementEditor = elementEditor;
            elementEditor.m_addCombo.setSelectedIndex(0);
        }

        @Override
        public void actionPerformed(ActionEvent e) {
            System.out.println(elementEditor.m_addCombo.getSelectedItem());
            //elementEditor.m_element = (BorealisElement) elementEditor.m_addCombo.getSelectedItem();
            //((PlotElement)elementEditor.m_element).changePlotParameters(elementEditor.m_addCombo.getSelectedIndex());
            //System.out.println("Changed to element with " + elementEditor.m_element.getContentSize());
            System.out.println("Selected index " + elementEditor.m_addCombo.getSelectedIndex());
            System.out.println("Selected in dropdown menu " + (BorealisElement) elementEditor.m_addCombo.getSelectedItem());
            elementEditor.m_elementTreeModel.setElement((BorealisElement) elementEditor.m_addCombo.getSelectedItem());
            elementEditor.showTreeUpdate();
        }
    }

    private class AddPlotButtonListener implements ActionListener
    {
        public AddPlotButtonListener()  {}

        public void actionPerformed(ActionEvent e)
        {
            final PlotElement element = new PlotElement("NewName" + m_newNameNumber++, "Migration time");

            JDialog diag = new JDialog(m_client, "Add New Plot To Simulation");
            GuiUtilities.centerOnParent(diag, true);

            Vector<FieldElement> childElements = new Vector<FieldElement>();

            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_BYTE_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_SHORT_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_INT_TYPE));

            ElementTreeEditor elementEditor = new ElementTreeEditor(
                    m_borealisModel, element, childElements);

            elementEditor.m_addCombo.removeAllItems();
            elementEditor.m_addCombo.addItem(new PlotElement("Migration time", "Migration time"));
            elementEditor.m_addCombo.addItem(new PlotElement("Input tuple latency", "Input latency"));
            elementEditor.m_addCombo.addItem(new PlotElement("Output tuple latency", "Output latency"));
            elementEditor.m_addCombo.setSelectedIndex(0);
            elementEditor.m_addCombo.addActionListener(new PlotDropMenuActionListener(elementEditor));

            elementEditor.getAddButton().setText("Add Plot");
            elementEditor.getAddButton().setActionCommand("AddPlot");
            elementEditor.getRemoveButton().setActionCommand("RemovePlot");

            JButton addPlotButton = new JButton("Add Plot");

            addPlotButton.addActionListener
                    (
                            e1 -> {
                                if (element.getParent()==null)
                                {
                                    if (m_borealisModel.getDocument().getAllSchemaNames().contains(
                                            element.getNameValue()))
                                    {
                                        JOptionPane.showConfirmDialog(null,
                                                "The plot name has already been used. Please change the name.",
                                                "Change Plot Name", JOptionPane.CLOSED_OPTION,
                                                JOptionPane.ERROR_MESSAGE);
                                    }
                                    else
                                    {
                                        m_borealisModel.getDocument().addPlot((PlotElement) elementEditor.m_addCombo.getSelectedItem());

                                        diag.dispose();
                                    }
                                }
                            }
                    );

            elementEditor.getOptionPanel().add(addPlotButton);

            diag.getContentPane().add(elementEditor);
            diag.setSize(new Dimension(500,200));
            diag.setVisible(true);
            m_networkTreeEditor.m_networkTreeModel.showTreeUpdate();
        }
    }

    private class AddNodeButtonListener implements ActionListener
    {
        int node_id = 1;
        public AddNodeButtonListener()  {}

        // Center on parent ( absolute true/false (exact center or 25% upper left) )
        public void centerOnParent(final Window child, final boolean absolute) {
            child.pack();
            boolean useChildsOwner = child.getOwner() != null ? ((child.getOwner() instanceof JFrame) || (child.getOwner() instanceof JDialog)) : false;
            final Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            final Dimension parentSize = useChildsOwner ? child.getOwner().getSize() : screenSize ;
            final Point parentLocationOnScreen = useChildsOwner ? child.getOwner().getLocationOnScreen() : new Point(0,0) ;
            final Dimension childSize = child.getSize();
            childSize.width = Math.min(childSize.width, screenSize.width);
            childSize.height = Math.min(childSize.height, screenSize.height);
            child.setSize(childSize);
            int x;
            int y;
            if ((child.getOwner() != null) && child.getOwner().isShowing()) {
                x = (parentSize.width - childSize.width) / 2;
                y = (parentSize.height - childSize.height) / 2;
                x += parentLocationOnScreen.x;
                y += parentLocationOnScreen.y;
            } else {
                x = (screenSize.width - childSize.width) / 2;
                y = (screenSize.height - childSize.height) / 2;
            }
            if (!absolute) {
                x /= 2;
                y /= 2;
            }
            child.setLocation(x, y);
        }

        public void actionPerformed(ActionEvent e)
        {
            final SimulationNodeElement element = new SimulationNodeElement("Node", m_newNameNumber++);

            JDialog diag = new JDialog(m_client, "Add New Node To Simulation");
            GuiUtilities.centerOnParent(diag, true);

            Vector<FieldElement> childElements = new Vector<FieldElement>();

            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_BYTE_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_SHORT_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_INT_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_LONG_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_SINGLE_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_DOUBLE_TYPE));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_STRING_TYPE, 20));
            childElements.add(new FieldElement("field_name",XmlConstants.SCHEMA_TIMESTAMP_TYPE));

            ElementTreeEditor elementEditor = new ElementTreeEditor(
                    m_borealisModel, element, childElements);
            //elementEditor.getComboBox().add(new SimulationNodeElement("new element"));

            elementEditor.m_addCombo.removeAllItems();
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_BYTE_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_SHORT_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_INT_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_LONG_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_SINGLE_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_DOUBLE_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_STRING_TYPE + " )");
            elementEditor.m_addCombo.addItem("field ( " + XmlConstants.SCHEMA_TIMESTAMP_TYPE + " )");
            elementEditor.m_addCombo.setSelectedIndex(0);

            elementEditor.getAddButton().setText("CPU cores");
            elementEditor.getAddButton().setActionCommand("AddField");
            elementEditor.getRemoveButton().setActionCommand("RemoveField");

            JButton addNodeButton = new JButton("Add Node");

            addNodeButton.addActionListener
                    (
                            e1 -> {
                                if (element.getParent()==null)
                                {
                                    //m_physicalModel.getDocument().addSimulationNode(element);

                                    BorealisGraphCellFactory fact = new BorealisGraphCellFactory();
                                    BoxElement elem = new BoxElement("Node " + node_id++, "");
                                    //m_physicalModel.add(elem, fact.createBox(elem));
                                    //m_physicalNetworkTreeModel.showTreeUpdate();
                                    diag.dispose();

                                    //JOptionPane.showConfirmDialog(null,
                                    //        "A node has been added.",
                                    //        "AddSchema", JOptionPane.CLOSED_OPTION,
                                    //        JOptionPane.INFORMATION_MESSAGE);
                                    }
                            }
                    );

            elementEditor.getOptionPanel().add(addNodeButton);
            diag.getContentPane().add(elementEditor);
            centerOnParent(diag, true);
            diag.setSize(new Dimension(500,200));
            diag.setVisible(true);
        }
    }


    private class AddQueryButtonListener implements ActionListener
    {
        public AddQueryButtonListener() {}

        public void actionPerformed(ActionEvent e)
        {
            BorealisElement element = new QueryElement("NewName" + m_newNameNumber++, "");

            JDialog diag = new JDialog(m_client, "Add New Query To Borealis");
            GuiUtilities.centerOnParent(diag, true);

            Vector<BorealisElement> childElements = new Vector<BorealisElement>();

            childElements.add(element);

            ElementTreeEditor elementEditor =
                   new ElementTreeEditor(m_borealisModel, element, childElements);

            elementEditor.getComboBox().setEnabled(false);
            elementEditor.getComboBox().setVisible(false);

            elementEditor.getAddButton().setText("Add Query");
            elementEditor.getAddButton().addActionListener
                (
                        e1 -> {
                            QueryElement elem = (QueryElement)elementEditor.m_addElements.get(0);
                            if (m_borealisModel.getDocument().getAllQueryNames().contains(
                                    elem.getNameValue()))
                            {
                                JOptionPane.showConfirmDialog(null,
                                        "The query name has already been used. Please change the name.",
                                        "Change Query Name", JOptionPane.CLOSED_OPTION,
                                        JOptionPane.ERROR_MESSAGE);
                            }
                            else
                            {   m_borealisModel.getDocument().addQuery(
                                    (QueryElement)(Element)elem.clone());

                                diag.dispose();
                            }
                        }
                );

            elementEditor.getRemoveButton().setVisible(false);

            diag.getContentPane().add(elementEditor);
            diag.setSize(new Dimension(500,200));
            diag.setVisible(true);
            m_networkTreeEditor.m_networkTreeModel.showTreeUpdate();
        }
    }


    private class NetworkTreeListener implements TreeSelectionListener
    {

        public NetworkTreeListener() {}

        public void valueChanged(TreeSelectionEvent e)
        {
            // first path element
            TreePath path = e.getPath();
            Object last = path.getLastPathComponent();

            if (last instanceof BorealisElement)
            {
                BorealisElement element = (BorealisElement) path.getLastPathComponent();
                setSelectedElement(element);
            }
            for (int i = 0; i < m_networkTreeEditor.m_networkTree.getRowCount(); i++) {
                m_networkTreeEditor.m_networkTree.expandRow(i);
            }
        }
    }
}


/////////////////////////  end EditorPanel.java  ///////////////////////////////
