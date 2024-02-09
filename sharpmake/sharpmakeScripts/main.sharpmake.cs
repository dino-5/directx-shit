using System.IO; // For Path.Combine
using Sharpmake; // Contains the entire Sharpmake object library.


// Both the library and the executable can share these base settings, so create
// a base class for both projects.
abstract class BaseSimpleLibraryProject : Project
{
    public BaseSimpleLibraryProject()
    {
        // Declares the target for which we build the project. This time we add
        // the additional OutputType fragment, which is a prebuilt fragment
        // that help us specify the kind of library output that we want.
        AddTargets(new Target(
             Platform.win64,
            DevEnv.vs2022,
            Optimization.Debug | Optimization.Release,
            OutputType.Dll | OutputType.Lib));
    }

    [Configure]
    public virtual void ConfigureAll(Project.Configuration conf, Target target)
    {
        // This is the name of the configuration. By default, it is set to
        // [target.Optimization] (so Debug or Release), but both the debug and
        // release configurations have both a shared and a static version so
        // that would not create unique configuration names.
        conf.Name = @"[target.Optimization] [target.OutputType]";

        // Gives a unique path for the project because Visual Studio does not
        // like shared intermediate directories.
        conf.ProjectPath = @"[project.SourceRootPath]";
    }
}

[Generate]
class ThirdPartyProject: BaseSimpleLibraryProject 
{
    public ThirdPartyProject()
    {
        Name = "ThirdParty";

        SourceRootPath = @"[project.SharpmakeCsPath]/../../source/third_party";

    }

    [Configure]
    public override void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");
        conf.IncludePaths.Add(@"[project.SourceRootPath]");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/imgui");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/imgui/backends");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/stb_image");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/tiny_obj_loader");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/fmt/include");
        conf.LibraryPaths.Add("C:/VulkanSDK/1.3.250.1/Lib");
        conf.IncludePaths.Add("C:/VulkanSDK/1.3.250.1/Include");
        conf.IncludePaths.Add("C:/VulkanSDK/1.3.250.1/shaderc");
        conf.LibraryFiles.Add("vulkan-1.lib");
        //conf.LibraryFiles.Add("shaderc.lib");
        //conf.LibraryFiles.Add("shaderc_combined.lib");
        conf.LibraryFiles.Add("shaderc_shared.lib");
        //conf.LibraryFiles.Add("shaderc_util.lib");

        if (target.OutputType == OutputType.Dll)
        {
            conf.Output = Configuration.OutputType.Dll;
        }
        else if (target.OutputType == OutputType.Lib)
        {
            conf.Output = Configuration.OutputType.Lib;
        }
        conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.CPP20);
    }

}

[Generate]
class EngineCommonProject : BaseSimpleLibraryProject
{
    public EngineCommonProject()
    {
        Name = "EngineCommon";
        SourceRootPath = @"[project.SharpmakeCsPath]/../../source/[project.Name]";
    }

    public override void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);


        // Sets the include path of the library. Those will be shared with any
        // project that adds this one as a dependency. (The executable here.)
        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");

        // The library wants LIBRARY_COMPILE defined when it compiles the
        // library, so that it knows whether it must use dllexport or
        // dllimport.
        //conf.Defines.Add("LIBRARY_COMPILE");

        if (target.OutputType == OutputType.Dll)
        {
            // We want this to output a shared library. (DLL)
            conf.Output = Configuration.OutputType.Dll;

            // This library project expects LIBRARY_DLL symbol to be defined
            // when used as a DLL. While we could define it in the executable,
            // it is better to put it as an exported define. That way, any
            // projects with a dependency on this one will have LIBRARY_DLL
            // automatically defined by Sharpmake.
            //conf.ExportDefines.Add("LIBRARY_DLL");

            // Exported defines are not necessarily defines as well, so we need
            // to add LIBRARY_DLL as an ordinary define too.
            //conf.Defines.Add("LIBRARY_DLL");
        }
        else if (target.OutputType == OutputType.Lib)
        {
            // We want this to output a static library. (LIB)
            conf.Output = Configuration.OutputType.Lib;
        }
        conf.IncludePaths.Add(@"[project.SourceRootPath]/fmt/include");
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP20);
        conf.AddPublicDependency<ThirdPartyProject>(target);
    }
}

// The library project.
[Generate]
class EngineGfxProject : BaseSimpleLibraryProject
{
    public EngineGfxProject()
    {
        Name = "EngineGfx";
        SourceRootPath = @"[project.SharpmakeCsPath]/../../source/[project.Name]";
    }

    public override void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);


        // Sets the include path of the library. Those will be shared with any
        // project that adds this one as a dependency. (The executable here.)
        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");

        // The library wants LIBRARY_COMPILE defined when it compiles the
        // library, so that it knows whether it must use dllexport or
        // dllimport.
        //conf.Defines.Add("LIBRARY_COMPILE");

        if (target.OutputType == OutputType.Dll)
        {
            // We want this to output a shared library. (DLL)
            conf.Output = Configuration.OutputType.Dll;

            // This library project expects LIBRARY_DLL symbol to be defined
            // when used as a DLL. While we could define it in the executable,
            // it is better to put it as an exported define. That way, any
            // projects with a dependency on this one will have LIBRARY_DLL
            // automatically defined by Sharpmake.
            //conf.ExportDefines.Add("LIBRARY_DLL");

            // Exported defines are not necessarily defines as well, so we need
            // to add LIBRARY_DLL as an ordinary define too.
            //conf.Defines.Add("LIBRARY_DLL");
        }
        else if (target.OutputType == OutputType.Lib)
        {
            // We want this to output a static library. (LIB)
            conf.Output = Configuration.OutputType.Lib;
        }
        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP20);
        conf.AddPublicDependency<ThirdPartyProject>(target);
        conf.AddPublicDependency<EngineCommonProject>(target);
    }
}

[Generate]
public class DemoProject: Project
{
    public DemoProject()
    {
        Name = "Demo";

        SourceRootPath = @"[project.SharpmakeCsPath]/../../source/Demo";

        AddTargets(new Target(
            Platform.win64,
            DevEnv.vs2022,
            Optimization.Debug | Optimization.Release));
    }

    [Configure]
    public void ConfigureAll(Project.Configuration conf, Target target)
    {
        // Specify where the generated project will be. Here we generate the
        // vcxproj in a /generated directory.
        conf.ProjectPath = @"[project.SourceRootPath]";
        conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.CPP20);
    }

    [Configure]
    public void Configure(Configuration conf, Target target)
    {
        conf.AddPublicDependency<EngineGfxProject>(target);
        conf.AddPublicDependency<EngineCommonProject>(target);
    }

}



[Generate]
public class DemoSolution : Solution
{
    public DemoSolution()
    {
        // The name of the solution.
        Name = "Dx12 Engine";

        // As with the project, define which target this solution builds for.
        // It's usually the same thing.
        AddTargets(new Target(
            Platform.win64,
            DevEnv.vs2022,
            Optimization.Debug | Optimization.Release));
    }

    // Configure for all 4 generated targets. Note that the type of the
    // configuration object is of type Solution.Configuration this time.
    // (Instead of Project.Configuration.)
    [Configure]
    public void ConfigureAll(Solution.Configuration conf, Target target)
    {
        // Puts the generated solution in the /generated folder too.
        conf.SolutionPath = @"[solution.SharpmakeCsPath]/../../generated";

        // Adds the project described by BasicsProject into the solution.
        // Note that this is done in the configuration, so you can generate
        // solutions that contain different projects based on their target.
        //
        // You could, for example, exclude a project that only supports 64-bit
        // from the 32-bit targets.
        conf.AddProject<EngineGfxProject>(target);
        conf.AddProject<EngineCommonProject>(target);
        conf.AddProject<DemoProject>(target);
        conf.AddProject<ThirdPartyProject>(target);
    }
}

public static class Main
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Sharpmake.Arguments arguments)
    {
        // Tells Sharpmake to generate the solution described by
        // BasicsSolution.
        arguments.Generate<DemoSolution>();
    }
}
