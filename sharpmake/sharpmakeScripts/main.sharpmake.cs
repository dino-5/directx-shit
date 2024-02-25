using Sharpmake; // Contains the entire Sharpmake object library.


// Both the library and the executable can share these base settings, so create
// a base class for both projects.
abstract class BaseSimpleLibraryProject : Project
{
    public BaseSimpleLibraryProject()
    {
        AddTargets(new Target(
             Platform.win64,
            DevEnv.vs2022,
            Optimization.Debug | Optimization.Release,
            OutputType.Dll | OutputType.Lib));
    }

    [Configure]
    public virtual void ConfigureAll(Project.Configuration conf, Target target)
    {
        conf.Name = @"[target.Optimization] [target.OutputType]";

        conf.ProjectPath = @"[project.SourceRootPath]";
    }
}

[Generate]
class ThirdPartyProject : BaseSimpleLibraryProject
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
        conf.IncludePaths.Add(@"[project.SourceRootPath]/../include");

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


        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");

        if (target.OutputType == OutputType.Dll)
        {
            conf.Output = Configuration.OutputType.Dll;
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

        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");

        if (target.OutputType == OutputType.Dll)
        {
            conf.Output = Configuration.OutputType.Dll;

        }
        else if (target.OutputType == OutputType.Lib)
        {
            conf.Output = Configuration.OutputType.Lib;
        }
        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/../include");
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP20);
        conf.AddPublicDependency<ThirdPartyProject>(target);
        conf.AddPublicDependency<EngineCommonProject>(target);
        conf.ReferencesByNuGetPackage.Add("Microsoft.Direct3D.D3D12", "1.611.2");
        conf.ReferencesByNuGetPackage.Add("Microsoft.Direct3D.DXC", "1.7.2308.12");
    }
}

[Generate]
public class DemoProject : Project
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
        conf.ReferencesByNuGetPackage.Add("Microsoft.Direct3D.D3D12", "1.611.2");
        conf.ReferencesByNuGetPackage.Add("Microsoft.Direct3D.DXC", "1.7.2308.12");
        conf.LibraryFiles.Add("dxcompiler");
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
