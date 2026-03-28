#include "CoreMinimal.h"
#include "McpCommand.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet/KismetNodeHelperLibrary.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphUtilities.h"

struct FNodeInfo
{
    FString Name;
    FString Description;
};

struct FNodeCategory
{
    FString CategoryName;
    TArray<FNodeInfo> Nodes;
};

class FMcpGetAvailableBlueprintNodesHandler : public FMcpCommandHandler
{
public:
    virtual FString GetCommandName() const override { return TEXT("get_available_blueprint_nodes"); }

    virtual FMcpCommandResult Execute(const TSharedPtr<FJsonObject>& Params) override
    {
        FString Category;
        Params->TryGetStringField(TEXT("category"), Category);

        FString SearchTerm;
        Params->TryGetStringField(TEXT("search"), SearchTerm);

        TArray<FNodeCategory> NodeCategories = BuildNodeCategories();

        TArray<TSharedPtr<FJsonValue>> NodesArray;

        for (const FNodeCategory& Cat : NodeCategories)
        {
            if (Category.IsEmpty() || Category == Cat.CategoryName)
            {
                for (const FNodeInfo& Node : Cat.Nodes)
                {
                    if (SearchTerm.IsEmpty() || Node.Name.Contains(SearchTerm) || Node.Description.Contains(SearchTerm))
                    {
                        TSharedPtr<FJsonObject> NodeInfo = MakeShareable(new FJsonObject);
                        NodeInfo->SetStringField(TEXT("name"), Node.Name);
                        NodeInfo->SetStringField(TEXT("category"), Cat.CategoryName);
                        NodeInfo->SetStringField(TEXT("description"), Node.Description);
                        NodesArray.Add(MakeShareable(new FJsonValueObject(NodeInfo)));
                    }
                }
            }
        }

        TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject);
        Result->SetArrayField(TEXT("nodes"), NodesArray);
        Result->SetNumberField(TEXT("count"), NodesArray.Num());

        TArray<TSharedPtr<FJsonValue>> CategoriesArray;
        for (const FNodeCategory& Cat : NodeCategories)
        {
            CategoriesArray.Add(MakeShareable(new FJsonValueString(Cat.CategoryName)));
        }
        Result->SetArrayField(TEXT("categories"), CategoriesArray);

        return FMcpCommandResult::Success(Result);
    }

private:
    TArray<FNodeCategory> BuildNodeCategories()
    {
        TArray<FNodeCategory> Categories;

        FNodeCategory FlowControl;
        FlowControl.CategoryName = TEXT("Flow Control");
        FlowControl.Nodes.Add({TEXT("Branch"), TEXT("Executes different logic based on a condition")});
        FlowControl.Nodes.Add({TEXT("Sequence"), TEXT("Executes outputs in sequential order")});
        FlowControl.Nodes.Add({TEXT("For Loop"), TEXT("Iterates a set number of times")});
        FlowControl.Nodes.Add({TEXT("For Loop with Break"), TEXT("Iterates with ability to break early")});
        FlowControl.Nodes.Add({TEXT("While Loop"), TEXT("Iterates while condition is true")});
        FlowControl.Nodes.Add({TEXT("Do Once"), TEXT("Executes only once until reset")});
        FlowControl.Nodes.Add({TEXT("Do N"), TEXT("Executes N times")});
        FlowControl.Nodes.Add({TEXT("Flip Flop"), TEXT("Alternates between two outputs")});
        FlowControl.Nodes.Add({TEXT("Gate"), TEXT("Controls execution flow like a gate")});
        FlowControl.Nodes.Add({TEXT("Multi Gate"), TEXT("Sequential execution with multiple outputs")});
        FlowControl.Nodes.Add({TEXT("Delay"), TEXT("Delays execution by specified time")});
        FlowControl.Nodes.Add({TEXT("Retriggerable Delay"), TEXT("Delay that can be restarted")});
        Categories.Add(FlowControl);

        FNodeCategory Variables;
        Variables.CategoryName = TEXT("Variables");
        Variables.Nodes.Add({TEXT("Set"), TEXT("Sets a variable value")});
        Variables.Nodes.Add({TEXT("Get"), TEXT("Gets a variable value")});
        Variables.Nodes.Add({TEXT("Set Array Elem"), TEXT("Sets an element in an array")});
        Variables.Nodes.Add({TEXT("Get Array Elem"), TEXT("Gets an element from an array")});
        Variables.Nodes.Add({TEXT("Array Add"), TEXT("Adds element to array")});
        Variables.Nodes.Add({TEXT("Array Remove"), TEXT("Removes element from array")});
        Variables.Nodes.Add({TEXT("Array Clear"), TEXT("Clears all elements")});
        Variables.Nodes.Add({TEXT("Array Length"), TEXT("Gets array length")});
        Variables.Nodes.Add({TEXT("Array Contains"), TEXT("Checks if array contains element")});
        Variables.Nodes.Add({TEXT("Array Find"), TEXT("Finds element index")});
        Categories.Add(Variables);

        FNodeCategory Math;
        Math.CategoryName = TEXT("Math");
        Math.Nodes.Add({TEXT("Add"), TEXT("Adds two values")});
        Math.Nodes.Add({TEXT("Subtract"), TEXT("Subtracts two values")});
        Math.Nodes.Add({TEXT("Multiply"), TEXT("Multiplies two values")});
        Math.Nodes.Add({TEXT("Divide"), TEXT("Divides two values")});
        Math.Nodes.Add({TEXT("Modulo"), TEXT("Modulo operation")});
        Math.Nodes.Add({TEXT("Power"), TEXT("Raises to power")});
        Math.Nodes.Add({TEXT("Square Root"), TEXT("Calculates square root")});
        Math.Nodes.Add({TEXT("Abs"), TEXT("Absolute value")});
        Math.Nodes.Add({TEXT("Min"), TEXT("Minimum of two values")});
        Math.Nodes.Add({TEXT("Max"), TEXT("Maximum of two values")});
        Math.Nodes.Add({TEXT("Clamp"), TEXT("Clamps value to range")});
        Math.Nodes.Add({TEXT("Lerp"), TEXT("Linear interpolation")});
        Math.Nodes.Add({TEXT("FInterp To"), TEXT("Float interpolation")});
        Math.Nodes.Add({TEXT("VInterp To"), TEXT("Vector interpolation")});
        Math.Nodes.Add({TEXT("RInterp To"), TEXT("Rotator interpolation")});
        Categories.Add(Math);

        FNodeCategory Logic;
        Logic.CategoryName = TEXT("Logic");
        Logic.Nodes.Add({TEXT("Equal"), TEXT("Checks equality")});
        Logic.Nodes.Add({TEXT("Not Equal"), TEXT("Checks inequality")});
        Logic.Nodes.Add({TEXT("Greater"), TEXT("Greater than comparison")});
        Logic.Nodes.Add({TEXT("Less"), TEXT("Less than comparison")});
        Logic.Nodes.Add({TEXT("Greater Equal"), TEXT("Greater or equal")});
        Logic.Nodes.Add({TEXT("Less Equal"), TEXT("Less or equal")});
        Logic.Nodes.Add({TEXT("AND"), TEXT("Logical AND")});
        Logic.Nodes.Add({TEXT("OR"), TEXT("Logical OR")});
        Logic.Nodes.Add({TEXT("NOT"), TEXT("Logical NOT")});
        Logic.Nodes.Add({TEXT("Select"), TEXT("Select between two values")});
        Logic.Nodes.Add({TEXT("Switch"), TEXT("Switch statement")});
        Categories.Add(Logic);

        FNodeCategory Actor;
        Actor.CategoryName = TEXT("Actor");
        Actor.Nodes.Add({TEXT("Spawn Actor"), TEXT("Spawns a new actor")});
        Actor.Nodes.Add({TEXT("Destroy Actor"), TEXT("Destroys an actor")});
        Actor.Nodes.Add({TEXT("Get Actor Location"), TEXT("Gets actor world location")});
        Actor.Nodes.Add({TEXT("Set Actor Location"), TEXT("Sets actor world location")});
        Actor.Nodes.Add({TEXT("Get Actor Rotation"), TEXT("Gets actor rotation")});
        Actor.Nodes.Add({TEXT("Set Actor Rotation"), TEXT("Sets actor rotation")});
        Actor.Nodes.Add({TEXT("Get Actor Scale"), TEXT("Gets actor scale")});
        Actor.Nodes.Add({TEXT("Set Actor Scale"), TEXT("Sets actor scale")});
        Actor.Nodes.Add({TEXT("Get All Actors of Class"), TEXT("Gets all actors of a class")});
        Actor.Nodes.Add({TEXT("Get Actor Components"), TEXT("Gets all components")});
        Categories.Add(Actor);

        FNodeCategory Communication;
        Communication.CategoryName = TEXT("Communication");
        Communication.Nodes.Add({TEXT("Event Dispatcher"), TEXT("Creates event dispatcher")});
        Communication.Nodes.Add({TEXT("Bind Event"), TEXT("Binds to event dispatcher")});
        Communication.Nodes.Add({TEXT("Unbind Event"), TEXT("Unbinds from event dispatcher")});
        Communication.Nodes.Add({TEXT("Cast To"), TEXT("Casts to specific type")});
        Communication.Nodes.Add({TEXT("Interface Call"), TEXT("Calls interface function")});
        Communication.Nodes.Add({TEXT("Get Player Controller"), TEXT("Gets player controller")});
        Communication.Nodes.Add({TEXT("Get Game Instance"), TEXT("Gets game instance")});
        Communication.Nodes.Add({TEXT("Get Game Mode"), TEXT("Gets game mode")});
        Categories.Add(Communication);

        FNodeCategory Input;
        Input.CategoryName = TEXT("Input");
        Input.Nodes.Add({TEXT("Axis Event"), TEXT("Axis input event")});
        Input.Nodes.Add({TEXT("Action Event"), TEXT("Action input event")});
        Input.Nodes.Add({TEXT("Key Event"), TEXT("Keyboard input event")});
        Input.Nodes.Add({TEXT("Touch Event"), TEXT("Touch input event")});
        Input.Nodes.Add({TEXT("Mouse Event"), TEXT("Mouse input event")});
        Categories.Add(Input);

        FNodeCategory Utility;
        Utility.CategoryName = TEXT("Utility");
        Utility.Nodes.Add({TEXT("Print String"), TEXT("Prints string to screen/log")});
        Utility.Nodes.Add({TEXT("Print Text"), TEXT("Prints text to screen")});
        Utility.Nodes.Add({TEXT("Make Literal"), TEXT("Creates literal value")});
        Utility.Nodes.Add({TEXT("Format String"), TEXT("Formats string with arguments")});
        Utility.Nodes.Add({TEXT("Get Time Seconds"), TEXT("Gets game time in seconds")});
        Utility.Nodes.Add({TEXT("Get Game Time"), TEXT("Gets game time")});
        Utility.Nodes.Add({TEXT("Get Delta Seconds"), TEXT("Gets frame delta time")});
        Categories.Add(Utility);

        return Categories;
    }
};

REGISTER_MCP_COMMAND(FMcpGetAvailableBlueprintNodesHandler)
