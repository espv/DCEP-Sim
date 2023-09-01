package borgui.xml;

import org.jdom2.Element;

import java.util.Iterator;
import java.util.List;
import java.util.Vector;


/**
 * A class that represent a Query Borealis Element
 */
public class PlotElement extends BorealisElement
{
    public static final String NAME = XmlConstants.PLOT_ELEMENT;
    public String type;
    public int index = 0;

    public PlotElement(String name, String type)
    {
        super(NAME);
        this.setNameValue(name);
        this.setType(type);
        this.type = type;
        this.setAttribute("Control-experiment-id", "");

        switch (this.type) {
            case "Migration time" -> {
                this.index = 0;
            }
            case "Input latency" -> {
                this.setAttribute("Query-id", "");
                this.setAttribute("Operator-name", "");
                this.setAttribute("Node-id", "");
                this.index = 1;
            }
            case "Output latency" -> {
                this.setAttribute("Query-id", "");
                this.setAttribute("Operator-name", "");
                this.setAttribute("Node-id", "");
                this.index = 2;
            }
            default -> {
                System.out.println("Invalid plot type " + type);
                System.exit(9);
            }
        }
    }

    public String toString() {
        return this.type;
    }


    public String getNameValue()
    {
        return this.getAttributeValue(XmlConstants.NAME_ATTRIBUTE);
    }


    public void setNameValue(String name)
    {
        this.setAttribute(XmlConstants.NAME_ATTRIBUTE, name);
    }

    public void setType(String type)
    {
        this.setAttribute(XmlConstants.FIELD_TYPE_ATTRIBUTE, type);
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
