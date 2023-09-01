/*
 * ReplicSetElement.java
 *
 * Created on February 6, 2006, 5:12 PM
 *
 * To change this template, choose Tools | Options and locate the template under
 * the Source Creation and Management node. Right-click the template and choose
 * Open. You can then make changes to the template in the Source Editor.
 */

package borgui.xml;

import org.jdom2.Element;

/**
 *
 * @author vivian
 */
public class ExperimentTaskSetElement extends BorealisElement
{
    public static final String NAME = XmlConstants.EXPERIMENTTASK_ELEMENT;


    /** Creates a new instance of ReplicSetElement */
    public ExperimentTaskSetElement(String name)
    {
        super(NAME);

        this.setNameValue(name);
    }


    public String getNameValue()
    {
        return this.getAttributeValue(XmlConstants.NAME_ATTRIBUTE);
    }


    public void setNameValue(String name)
    {
        this.setAttribute(XmlConstants.NAME_ATTRIBUTE, name);
    }
}


////////////////////////  end  ReplicSetElement.java  //////////////////////////
