package borgui.xml;

import org.jdom2.Element;

import java.util.ArrayList;
import java.util.List;
import borgui.common.*;

/**
 * A class that represent a Network Borealis Element
 */

public class NetworkElement extends BorealisElement
{
    public static String NAME = XmlConstants.NETWORK_ELEMENT;
    public static List<QueryElement> queryElements = new ArrayList<>();
    public static List<ExperimentElement> experimentElements = new ArrayList<>();
    public static List<PlotElement> plotElements = new ArrayList<>();
    public static List<DatasetElement> datasetElements = new ArrayList<>();


    public NetworkElement()
    {
        super(NAME);
    }


    public NetworkElement(Element element)
    {
        super(NAME, element);
    }


    public List getAllSchemas()
    {
        return this.getChildren(XmlConstants.SCHEMA_ELEMENT);
    }

    public NetworkParameterElement getNetworkParameters()
    {
        if (!this.getChildren(XmlConstants.NETWORKPARAMETER_ELEMENT).isEmpty()) {
            return (NetworkParameterElement) this.getChildren(XmlConstants.NETWORKPARAMETER_ELEMENT).get(0);
        }

        return null;
    }


    public SchemaElement getSchema(String name)
    {
        return (SchemaElement) XmlHandler.findElement(this,
                                         XmlConstants.SCHEMA_ELEMENT,
                                         XmlConstants.NAME_ATTRIBUTE, name);
    }

    public void setNetworkParameters(NetworkParameterElement networkParameterElement)
    {
        this.addContent(networkParameterElement);
    }

    public boolean addSchema(SchemaElement schema)
    {
        this.addContent(schema);
        return true;
    }

    public boolean addSimulationNode(SimulationNodeElement simulationNode)
    {
        this.addContent(simulationNode);
        return true;
    }


    public boolean addDataset(DatasetElement datasetElement)
    {
        datasetElements.add(datasetElement);
        this.addContent(datasetElement);
        return true;
    }


    public boolean removeSchema(String name)
    {
        Element element = this.getSchema(name);

        if (element == null)
        {   return false;
        }

        return this.removeContent(element);
    }


    public List getAllInputs()
    {
        return this.getChildren(XmlConstants.INPUT_ELEMENT);
    }


    public InputElement getInput(String name)
    {
        return (InputElement) XmlHandler.findElement(this,
                                     XmlConstants.INPUT_ELEMENT,
                                     XmlConstants.INPUT_STREAM_ATTRIBUTE, name);
    }


    public boolean addInput(InputElement input)
    {
        this.addContent(input);
        return true;
    }


    public boolean removeInput(String name)
    {
        Element element = this.getInput(name);

        if (element == null)
        {
            return false;
        }

        return this.removeContent(element);
    }


    public List getAllOutputs()
    {
        return this.getChildren(XmlConstants.OUTPUT_ELEMENT);
    }


    public OutputElement getOutput(String name)
    {
        return (OutputElement) XmlHandler.findElement(this,
                                         XmlConstants.OUTPUT_ELEMENT,
                                         XmlConstants.OUTPUT_STREAM_ATTRIBUTE,
                                                      name);
    }


    public boolean  addOutput(OutputElement output)
    {
        this.addContent(output);
        return true;
    }


    public boolean  removeOutput(String name)
    {
        Element element = this.getOutput(name);

        if (element == null)
        {   return false;
        }

        return this.removeContent(element);
    }


    public boolean  removeOutElement(BoxElement boxElement, String name)
    {
        return boxElement.removeOut(name);
    }


    public List  getAllBoxes()
    {
        return  this.getChildren(XmlConstants.BOX_ELEMENT);
    }


    public List  getAllQueries()
    {
        return this.getChildren(XmlConstants.QUERY_ELEMENT);
    }


    public List  getAllDatasets()
    {
        return this.getChildren(XmlConstants.DATASET_ELEMENT);
    }


    public List  getAllExperiments()
    {
        return this.getChildren(XmlConstants.EXPERIMENT_ELEMENT);
    }


    public List  getAllPlots()
    {
        return this.getChildren(XmlConstants.PLOT_ELEMENT);
    }


    public QueryElement  getQuery(String name)
    {
        return (QueryElement) XmlHandler.findElement(this,
                                        XmlConstants.QUERY_ELEMENT,
                                        XmlConstants.NAME_ATTRIBUTE, name);
    }


    public boolean addQuery(QueryElement query)
    {
        queryElements.add(query);
        this.addContent(query);
        return true;
    }


    public boolean removeExperiment(ExperimentElement experimentElement)
    {
        experimentElements.remove(experimentElement);
        this.removeContent(experimentElement);
        return true;
    }


    public boolean addExperiment(ExperimentElement experimentElement)
    {
        experimentElements.add(experimentElement);
        this.addContent(experimentElement);
        return true;
    }


    public boolean addPlot(PlotElement plotElement)
    {
        plotElements.add(plotElement);
        this.addContent(plotElement);
        return true;
    }


    public boolean addExperimentTask(ExperimentTaskSetElement taskSetElement)
    {
        this.addContent(taskSetElement);
        return true;
    }


    public boolean removeQuery(String name)
    {
        Element element = this.getQuery(name);

        if (element == null)
        {   return false;
        }
        for (int i = 0; i < queryElements.size(); i++) {
            if (queryElements.get(i).getName().equals(element.getName())) {
                queryElements.remove(i);
                break;
            }
        }

        return this.removeContent(element);
    }
}


///////////////////////  end  NetworkElement.java  /////////////////////////////
