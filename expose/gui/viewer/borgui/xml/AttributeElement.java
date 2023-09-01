package borgui.xml;

import org.jdom2.Element;

/**
 * A class that represent a Attribute Borealis Element
 */

public class AttributeElement extends BorealisElement
{
    public static final String NAME = XmlConstants.ATTRIBUTE_ELEMENT;


    public AttributeElement(String name, String type, String value, String valueType)
    {
        super(NAME);
        this.setNameValue(name);
        this.setType(type);
        this.setValue(value);
        this.setValueType(valueType);
    }


    public AttributeElement(Element element)
    {
        super(NAME, element);
    }


    public String getNameValue()
    {
        return this.getAttributeValue(XmlConstants.NAME_ATTRIBUTE);
    }

    public String getType()
    {
        return this.getAttributeValue(XmlConstants.FIELD_TYPE_ATTRIBUTE);
    }

    public String getValueType()
    {
        return this.getAttributeValue(XmlConstants.FIELD_VALUETYPE_ATTRIBUTE);
    }

    public void setValueType(String valueType)
    {
        this.setAttribute(XmlConstants.FIELD_VALUETYPE_ATTRIBUTE, valueType);
    }


    public void setType(String type)
    {
        this.setAttribute(XmlConstants.FIELD_TYPE_ATTRIBUTE, type);
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


/////////////////////  end  AttributeElement.java  /////////////////////////////
