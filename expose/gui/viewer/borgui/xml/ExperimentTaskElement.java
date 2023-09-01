package borgui.xml;

import org.jdom2.Element;

import java.util.*;


/**
 * A class that represent a Query Borealis Element
 */
public class ExperimentTaskElement extends BorealisElement
{
    public static final String NAME = XmlConstants.EXPERIMENTTASK_ELEMENT;
    int id;
    static int cnt_id = 0;
    List<ExperimentTaskElement> experimentTaskElements = new ArrayList<>();
    List<Object> args;
    int node = -1;
    List<ParameterElement> parameterElements = new ArrayList<>();
    public static Map<Integer, ExperimentTaskElement> deployedQueries = new HashMap<>();


    public ExperimentTaskElement(String cmd, List<Object> args)
    {
        super(cmd);
        this.args = args;
        this.id = ++cnt_id;
    }

    public void setNode(int node) {
        this.node = node;
    }

    public int getId() {return id;}

    public List<ParameterElement> getParameters() {
        return parameterElements;
    }

    public String getNameValue()
    {
        return this.getAttributeValue(XmlConstants.NAME_ATTRIBUTE);
    }

    public void buildElementFromTask() {
        //m_borealisModel.getDocument().addExperimentTask((ExperimentTaskSetElement) element.clone());
        String task = getName();
        String strNodeId = Integer.toString(node);
        if (node == 0) {
            strNodeId = "coordinator";
        }
        setAttribute("node", strNodeId);
        if (task.equals("deployQueries")) {
            setAttribute("queryId", Integer.toString((int) args.get(0)));
            setAttribute("uniqueQueryId", Integer.toString((int) args.get(1)));
            deployedQueries.put((int) args.get(1), this);
        } else if (task.equals("addPotentialHost")) {
            List<Integer> nodeIds = (List<Integer>) args.get(0);
            StringBuilder strNodeIds = new StringBuilder();
            for (int nodeId : nodeIds) {
                strNodeIds.append(nodeId).append(",");
            }
            setAttribute("nodeId", strNodeIds.toString());
            setAttribute("uniqueQueryId", Integer.toString((int) args.get(1)));
        } else if (task.equals("addSourceNode")) {
            List<Integer> nodeIds = (List<Integer>) args.get(0);
            StringBuilder strNodeIds = new StringBuilder();
            for (int nodeId : nodeIds) {
                strNodeIds.append(nodeId).append(",");
            }
            setAttribute("nodeId", strNodeIds.toString());
            setAttribute("uniqueQueryId", Integer.toString((int) args.get(1)));
        } else if (task.equals("addSinkNode")) {
            List<Object> nodeIds = (List<Object>) args.get(0);
            StringBuilder strNodeIds = new StringBuilder();
            for (Object nodeId : nodeIds) {
                strNodeIds.append(nodeId).append(",");
            }
            setAttribute("nodeId", strNodeIds.toString());
            setAttribute("uniqueQueryId", Integer.toString((int) args.get(1)));
        } else if (task.equals("sendDsAsStream") || task.equals("sendDsAsSpecificStream")) {
            setAttribute("datasetId", Integer.toString((int) args.get(0)));
            setAttribute("iterations", Integer.toString((int) args.get(1)));
        } else if (task.equals("sendNRowsDsAsSpecificStream")) {
            setAttribute("datasetId", Integer.toString((int) args.get(0)));
            setAttribute("iterations", Integer.toString((int) args.get(1)));
            setAttribute("number-rows", Integer.toString((int) args.get(2)));
        } else if (task.equals("disconnectNode")) {
            setAttribute("msBeforeDisconnect", Integer.toString((int) args.get(0)));
        } else if (task.equals("retEndOfStream")) {
            setAttribute("ms", Integer.toString((int) args.get(0)));
        } else if (task.equals("retSinkEndOfStream")) {
            setAttribute("queryId", Integer.toString((int) args.get(0)));
            setAttribute("ms", Integer.toString((int) args.get(1)));
        } else if (task.equals("retWhenReceived")) {
            setAttribute("tuples", Integer.toString((int) args.get(0)));
        } else if (task.equals("wait")) {
            setAttribute("ms", Integer.toString((int) args.get(0)));
        } else if (task.equals("loopTasks")) {
            setAttribute("iterations", Integer.toString((int) args.get(0)));
            addContent(new TaskFlowSetElement());
        } else if (task.equals("batchTasks")) {
            addContent(new TaskFlowSetElement());
        } else if (task.equals("adapt")) {
            setAttribute("queryId", Integer.toString((int) args.get(0)));
            setAttribute("nodeId", Integer.toString((int) args.get(1)));
            setAttribute("migrationMode", (String) args.get(2));
        }

        for (ParameterElement parameterElement : parameterElements) {
            //addContent(parameterElement);
        }
    }

    public void buildNewElement() {
        //m_borealisModel.getDocument().addExperimentTask((ExperimentTaskSetElement) element.clone());
        String task = getName();
        setAttribute("node", Integer.toString(node));
        //addContent(new ParameterElement("node", Integer.toString(node)));
        if (task.equals("deployQueries")) {
            //this.addContent(new ArrayList<>(){})
            //addContent(new ParameterElement("queryId", ""));
            //addContent(new ParameterElement("uniqueQueryId", ""));
            setAttribute("queryId", "");
            setAttribute("uniqueQueryId", "");
        } else if (task.equals("addPotentialHost")) {
            setAttribute("nodeId", "");
            setAttribute("uniqueQueryId", "");
        } else if (task.equals("addSourceNode")) {
            setAttribute("nodeId", "");
            setAttribute("uniqueQueryId", "");
        } else if (task.equals("addSinkNode")) {
            setAttribute("nodeId", "");
            setAttribute("uniqueQueryId", "");
        } else if (task.equals("sendDsAsStream") || task.equals("sendDsAsSpecificStream")) {
            setAttribute("datasetId", "");
            setAttribute("iterations", "");
        } else if (task.equals("sendNRowsDsAsSpecificStream")) {
            setAttribute("datasetId", "");
            setAttribute("iterations", "");
            setAttribute("number-rows", "");
        } else if (task.equals("disconnectNode")) {
            setAttribute("msBeforeDisconnect", "");
            setAttribute("msBeforeNotifyDisconnect", "");
        } else if (task.equals("retEndOfStream")) {
            setAttribute("ms", "");
        } else if (task.equals("retSinkEndOfStream")) {
            setAttribute("queryId", "");
            setAttribute("ms", "");
        } else if (task.equals("retWhenReceived")) {
            setAttribute("tuples", "");
        } else if (task.equals("wait")) {
            setAttribute("ms", "");
        } else if (task.equals("loopTasks")) {
            setAttribute("iterations", "");
            addContent(new TaskFlowSetElement());
        } else if (task.equals("batchTasks")) {
            addContent(new TaskFlowSetElement());
        } else if (task.equals("adapt")) {
            setAttribute("queryId", "");
            setAttribute("nodeId", "");
            setAttribute("migrationMode", "");
        }
    }


    public static List<Map<String, Object>> exportExperimentList(TaskFlowSetElement rootTask) {
        List<Map<String, Object>> ret = new ArrayList<>();
        for (Element element : rootTask.getChildren()) {
            Map<String, Object> mapTask = new HashMap<>();
            ret.add(mapTask);
            ExperimentTaskElement taskElement = (ExperimentTaskElement) element;
            List<Object> arguments = new ArrayList<>();
            mapTask.put("arguments", arguments);
            String task = taskElement.getName();
            mapTask.put("task", task);
            mapTask.put("node", taskElement.node);

            // TODO: actually store the fields
            // TODO: currently, they are just hard-coded
            Map<String, String> fields = new HashMap<>();
            List<ParameterElement> parameterElements = taskElement.getParameters();
            //ParameterElement nodeParameterElement = parameterElements.get(0);
            String nodeId = taskElement.getAttributeValue("node");
            if (nodeId.equals("coordinator")) {
                mapTask.put("node", nodeId);
            } else {
                mapTask.put("node", Integer.parseInt(nodeId));
            }
            //for (int i = 1; i < parameterElements.size(); i++) {
                //ParameterElement parameterElement = parameterElements.get(i);
                //fields.put(parameterElement.getNameValue(), parameterElement.getValue());

            if (task.equals("deployQueries")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("queryId")));
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("uniqueQueryId")));
            } else if (task.equals("addPotentialHost")) {
                String[] l = taskElement.getAttributeValue("nodeId").split(",");
                List<Integer> n = new ArrayList<>();
                for (String s : l) {
                    if (s.isEmpty()) {
                        continue;
                    }
                    n.add(Integer.parseInt(s));
                }
                arguments.add(n);
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("uniqueQueryId")));
            } else if (task.equals("addSourceNode")) {
                String[] l = taskElement.getAttributeValue("nodeId").split(",");
                List<Integer> n = new ArrayList<>();
                for (String s : l) {
                    if (s.isEmpty()) {
                        continue;
                    }
                    n.add(Integer.parseInt(s));
                }
                arguments.add(n);
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("uniqueQueryId")));
            } else if (task.equals("addSinkNode")) {
                String[] l = taskElement.getAttributeValue("nodeId").split(",");
                List<Object> n = new ArrayList<>();
                for (String s : l) {
                    if (s.isEmpty()) {
                        continue;
                    }

                    try {
                        n.add(Integer.parseInt(s));
                    } catch (Exception e) {
                        n.add(s);
                    }
                }
                arguments.add(n);
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("uniqueQueryId")));
            } else if (task.equals("sendDsAsStream") || task.equals("sendDsAsSpecificStream")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("datasetId")));
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("iterations")));
            } else if (task.equals("sendNRowsDsAsSpecificStream")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("datasetId")));
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("iterations")));
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("number-rows")));
            } else if (task.equals("disconnectNode")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("msBeforeDisconnect")));
                //arguments.add(fields);
            } else if (task.equals("retEndOfStream")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("ms")));
            } else if (task.equals("retSinkEndOfStream")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("queryId")));
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("ms")));
            } else if (task.equals("retWhenReceived")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("tuples")));
            } else if (task.equals("wait")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("ms")));
            } else if (task.equals("loopTasks")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("iterations")));
                TaskFlowSetElement subRoot = (TaskFlowSetElement) taskElement.getChildren().get(0);
                arguments.add(exportExperimentList(subRoot));
            } else if (task.equals("batchTasks")) {
                // task list
                TaskFlowSetElement subRoot = (TaskFlowSetElement) taskElement.getChildren().get(0);
                arguments.add(exportExperimentList(subRoot));
            } else if (task.equals("adapt")) {
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("queryId")));
                arguments.add(Integer.parseInt(taskElement.getAttributeValue("nodeId")));
                arguments.add(taskElement.getAttributeValue("migrationMode"));
            }
        }
        return ret;
    }


    public void setNameValue(String name)
    {
        this.setAttribute(XmlConstants.NAME_ATTRIBUTE, name);
    }


    public void addTask(ExperimentTaskElement taskElement) {
        this.experimentTaskElements.add(taskElement);
    }


//    public List<QueryElement>  getAllBoxes()
//    {
//        return this.getChildren(XmlConstants.BOX_ELEMENT);
//    }


    public List<BoxElement>  getAllBoxes()
    {
        List<BoxElement>  elements = new Vector<BoxElement>();
        List   boxes = this.getChildren(XmlConstants.BOX_ELEMENT);

        for(Iterator i = boxes.iterator(); i.hasNext();)
        {
            elements.add((BoxElement)i.next());
        }

        return elements;
    }


    public Vector<String> getAllBoxNames()
    {
        Vector<String>  names = new Vector<String>();
        List            boxes = this.getChildren(XmlConstants.BOX_ELEMENT);

        for(Iterator i = boxes.iterator(); i.hasNext();)
        {
            BoxElement child = (BoxElement)i.next();
            names.add(child.getNameValue());
        }

        return names;
    }


    public BoxElement getBox(String name)
    {
        return (BoxElement) XmlHandler.findElement(this,
                XmlConstants.BOX_ELEMENT,
                XmlConstants.NAME_ATTRIBUTE,
                name);
    }


    public boolean addBox(BoxElement box)
    {
        this.addContent(box);
        return true;
    }


    public boolean removeBox(String name)
    {
        Element elem = this.getBox(name);

        if (elem == null)
        {
            return false;
        }

        return this.removeContent(elem);
    }


    //public List getAllTables()
    //{
    //    return this.getChildren(XmlConstants.TABLE_ELEMENT);
    //}


    public List<TableElement>  getAllTables()
    {
        List<TableElement>  elements = new Vector<TableElement>();
        List   tables = this.getChildren(XmlConstants.TABLE_ELEMENT);

        for(Iterator i = tables.iterator(); i.hasNext();)
        {
            elements.add((TableElement)i.next());
        }

        return elements;
    }


    public Vector<String> getAllTableNames()
    {
        Vector<String>  names = new Vector<String>();
        List tables = this.getChildren(XmlConstants.TABLE_ELEMENT);

        for (Iterator i = tables.iterator(); i.hasNext();)
        {
            TableElement child = (TableElement)i.next();
            names.add(child.getNameValue());
        }

        return names;
    }


    public TableElement getTable(String name)
    {
        return (TableElement)XmlHandler.findElement(this,
                XmlConstants.TABLE_ELEMENT,
                XmlConstants.NAME_ATTRIBUTE,
                name);
    }


    public boolean addTable(TableElement table)
    {
        this.addContent(table);
        return true;
    }


    public boolean removeTable(String name)
    {
        Element element = this.getTable(name);

        if (element == null)
        {
            return false;
        }

        return this.removeContent(element);
    }
}

//////////////////////////  end QueryElement.java  /////////////////////////////
