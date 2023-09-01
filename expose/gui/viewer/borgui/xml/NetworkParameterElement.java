package borgui.xml;


/**
 * A class that represent a Parameter Borealis Element
 */

public class NetworkParameterElement extends BorealisElement
{
    public static final String NAME = XmlConstants.NETWORKPARAMETER_ELEMENT;


    public NetworkParameterElement(String bw_migration, String bw_tuples, String lat_migration, String lat_tuples)
    {
        super(NAME);
        addContent(new ParameterElement("Migration bandwidth", bw_migration));
        addContent(new ParameterElement("Tuples bandwidth", bw_tuples));
        addContent(new ParameterElement("Migration latency", lat_migration));
        addContent(new ParameterElement("Tuples latency", lat_tuples));
    }

    public String getNameValue()
    {
        return this.getAttributeValue(XmlConstants.NAME_ATTRIBUTE);
    }


    public void setNameValue(String value)
    {
        this.setAttribute(XmlConstants.NAME_ATTRIBUTE, value);
    }


    public String getValue()
    {
        return this.getAttributeValue(XmlConstants.PARAMETER_VALUE_ATTRIBUTE);
    }


    public void setValue(String value)
    {
        this.setAttribute(XmlConstants.PARAMETER_VALUE_ATTRIBUTE, value);
    }
}


/////////////////////  end  ParameterElement.java  /////////////////////////////
