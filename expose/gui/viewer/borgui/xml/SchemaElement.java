package borgui.xml;

import org.jdom2.Element;
import java.util.List;
import borgui.common.*;

/**
 * A class that represent a Schema Borealis Element
 */
public class SchemaElement extends BorealisElement
{
    public final static String NAME = XmlConstants.SCHEMA_ELEMENT;


    public SchemaElement()
    {
        super(NAME);

        this.setNameValue("NewName0");
        this.setStreamId("0");
        this.addContent(new ParameterElement("Stream ID", ""));
    }

    public SchemaElement(String name)
    {
        super(NAME);
        this.setNameValue(name);
        this.setStreamId("0");
        this.addContent(new ParameterElement("Stream ID", ""));
    }

    public SchemaElement(String name, String streamId)
    {
        super(NAME);
        this.setNameValue(name);
        this.setStreamId(streamId);
        this.addContent(new ParameterElement("Stream ID", streamId));
    }


    public SchemaElement(Element element)
    {
        super(NAME, element);
    }


    public String getNameValue()
    {
        return this.getAttributeValue(XmlConstants.NAME_ATTRIBUTE);
    }


    public String getStreamId()
    {
        return this.getAttributeValue(XmlConstants.STREAM_ATTRIBUTE);
    }


    public void setNameValue(String name)
    {
        this.setAttribute(XmlConstants.NAME_ATTRIBUTE, name);
    }


    public void setStreamId(String streamId)
    {
        this.setAttribute(XmlConstants.STREAM_ATTRIBUTE, streamId);
    }


    public List getAllFields()
    {
        return this.getChildren(XmlConstants.FIELD_ELEMENT);
    }


    public FieldElement getField(String name)
    {
        return (FieldElement)XmlHandler.findElement(this,
                                                    XmlConstants.FIELD_ELEMENT,
                                                    XmlConstants.NAME_ATTRIBUTE,
                                                    name);
    }


    public boolean addField(FieldElement field)
    {
        this.addContent(field);
        return true;
    }


    public boolean removeField(String name)
    {
        Element element = this.getField(name);

        if (element == null)
        {   return false;
        }

        return this.removeContent(element);
    }
}


/////////////////////////  end  SchemaElement.java  ////////////////////////////
