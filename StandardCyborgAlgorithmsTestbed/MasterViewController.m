//
//  MasterViewController.m
//  StandardCyborgGeometryTestbed
//
//  Created by Aaron Thompson on 3/28/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

#import "MasterViewController.h"

@implementation MasterViewController {
    NSArray *_viewControllerNames;
    NSArray *_viewControllerClassNames;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _viewControllerNames =
    @[
      @"Distances and Angles",
      @"Plane Removal",
      @"Scenekit",
      
      ];
    _viewControllerClassNames =
    @[
      @"DistanceAngleViewController",
      @"PlaneRemovalViewController",
      @"ScenekitTestbedViewController",
      
      ];
}

- (void)viewWillAppear:(BOOL)animated
{
    self.clearsSelectionOnViewWillAppear = self.splitViewController.isCollapsed;
    
    [super viewWillAppear:animated];
}

// MARK: - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([[segue identifier] isEqualToString:@"showDetail"]) {
        NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
        NSString *className = _viewControllerClassNames[indexPath.row];
        
        UIStoryboard *storyboard = [UIStoryboard storyboardWithName:className bundle:nil];
        UIViewController *selectedVC = [storyboard instantiateInitialViewController];
        
        selectedVC.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
        selectedVC.navigationItem.leftItemsSupplementBackButton = YES;
        
        [[segue destinationViewController] setViewControllers:@[selectedVC]];
    }
}

// MARK: - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [_viewControllerNames count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"Cell" forIndexPath:indexPath];
    
    cell.textLabel.text = _viewControllerNames[indexPath.row];
    
    return cell;
}

@end
